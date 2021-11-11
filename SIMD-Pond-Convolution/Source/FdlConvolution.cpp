/*
  ==============================================================================

    FdlConvolution.cpp
    Created: 11 Nov 2021 1:36:59pm
    Author:  jamie

  ==============================================================================
*/

#include "FdlConvolution.h"


void FdlConvolver::process(int cachedInputBufferPointer, 
                           int cachedOuputBufferPointer,
                           bool shouldUseExternalBuffer,
                           juce::AudioBuffer<float>* _inputBuffer)
{
    // TODO -- IF WE GOT ENOUGH INPUT -> THEN PROCESS, IF NOT, JUST READ FROM OUTPUT. 

    // Iterate over each channel...
    for (int channel = 0; channel < bufferToProcess.getNumChannels(); channel++)
    {
        auto* bufferToProcessData = bufferToProcess.getWritePointer(channel);

        // Clear input buffer and output buffers...
        fillWithZeros(bufferToProcessData, mFftSize);
        fillWithZeros(reinterpret_cast<float*>(outputData.data()), mFftSize);

        // Copy unwrapped data from the input buffer.
        inputBuffer->getFromCircularBuffer(channel, mFftSize, bufferToProcess, true, cachedInputBufferPointer);

        // Do the FFT on the new info, and insert it into the frequency delay line. 
        fft.forward(bufferToProcessData, interleavedComplex.data());

        // Deinterleave the data from pffft. 
        deinterleave(reinterpret_cast<float*>(fdl[mWritePosition][channel].data()),
            reinterpret_cast<float*>(interleavedComplex.data()), mFftSize);


        float tempA, tempB; // helper floats for the convolution, allocating here for efficiency!
        // Iterate over all active slots in the FDL, and perform the relevant Cmadd operations. 
        // This adds the info into the outputData complex vector.
        for (int slot = 0; slot < numActiveSlots; slot++)
        {
            // TODO try deinterleaved, but with that simd pragma. 
            const int slotToUse = modulo(mReadPosition - slot, numActiveSlots);
            complexMultiplication(reinterpret_cast<float*>(outputData.data()),
                                 (fdl[slotToUse][channel].data()),
                                 (irSegments[slot]->complexPartition[channel].data()),
                                 mFftSize,
                                 shouldUseOptimizedFunction, 
                                 tempA, tempB);
        }

        // Interleave the data into complex.h format for pffft. 
        interleave(reinterpret_cast<float*>(interleavedComplex.data()),
            reinterpret_cast<float*>(outputData.data()), mFftSize);

        // Perform the inverse FFT, on the output. 
        fft.inverse(interleavedComplex.data(), bufferToProcessData);

        // Scale the output!
        float gain = 8.0f / (sqrtf(hostSampleRate) * mFftSize);
        bufferToProcess.applyGain(channel, 0, bufferToProcess.getNumSamples(), gain);

        // Return the output to the output buffer for reading. 
        // TODO, we might be able to bin off the input and output buffers entirely, not sure. 
        outputBuffer->addToCircularBuffer(channel, mFftSize, bufferToProcessData, cachedOuputBufferPointer);
    }

    // Advance the FDL read and write pointers. 
    advanceWritePosition();
    advanceReadPosition();
}

void FdlConvolver::setup()
{
    const int numChannels = 2;

    irSampleCounter.resize(numChannels);

    irSegments.clear();
    temporarySegment.setSize(2, headSize, false, true, false);
    decayCurve.resize(headSize);
    outputData.resize(mFftSize / 2);
    interleavedComplex.resize(mFftSize / 2);
    bufferToProcess.setSize(numChannels, mFftSize);

    // Load impulse responses...
    bool shouldResample = hostSampleRate != sampleRateOfImpulseResponses;
    int largestNumSamples = -1;

    std::vector<float> rmsValues(impulseResponsesToLoad.size());

    for (int i = 0; i < impulseResponsesToLoad.size(); i++)
    {
        auto impulseResponseBuffer = wavFileLoader(impulseResponsesToLoad[i].name, impulseResponsesToLoad[i].size);

        normalizeBuffer(impulseResponseBuffer);
        rmsValues[i] = (impulseResponseBuffer.getRMSLevel(0, 0, impulseResponseBuffer.getNumSamples()));

        if (shouldResample)
        {
            irArray.push_back(resampleBuffer(impulseResponseBuffer, sampleRateOfImpulseResponses, hostSampleRate));
        }
        else
        {
            irArray.push_back(impulseResponseBuffer);
        }



        int nextMultipleOfHeadSize = irArray[i].getNumSamples() + (headSize - irArray[i].getNumSamples() % headSize);
        irArray[i].setSize(irArray[i].getNumChannels(), nextMultipleOfHeadSize, true, true, false);
        largestNumSamples = std::max(largestNumSamples, nextMultipleOfHeadSize);
    }

    float minRMS = 10000.0f;
    for (int i = 0; i < rmsValues.size(); i++)
        minRMS = juce::jmin(minRMS, rmsValues[i]);

    for (int i = 0; i < impulseResponsesToLoad.size(); i++)
    {
        float smallestDB = juce::Decibels::gainToDecibels(minRMS);
        float targetRMSdB = juce::Decibels::gainToDecibels(rmsValues[i]);
        float differenceDB = smallestDB - targetRMSdB;
        float targetGain = juce::Decibels::decibelsToGain(differenceDB);
        DBG(targetGain);
        irArray[i].applyGain(targetGain);
    }

    largestNumSamples = pffft::nextPowerOfTwo(largestNumSamples);
    numSlots = largestNumSamples / headSize;
    numActiveSlots = numSlots;

    if (modulo(largestNumSamples, headSize) != 0)
        jassertfalse; // Should have non-extraneous samples. 

    // Create FDL structure. 
    fdl.resize(numSlots);
    irSegments.resize(numSlots);
    for (int slot = 0; slot < numSlots; slot++)
    {
        fdl[slot].resize(numChannels);
        for (int channel = 0; channel < numChannels; channel++)
        {
            fdl[slot][channel].resize(mFftSize);
        }
    }
}

void FdlConvolver::updateImpulseResponse(float decayValue, int _targetIr)
{
    if (_targetIr != irToUse)
        irToUse = _targetIr;

    const int numChannels = irArray[irToUse].getNumChannels();
    const int numSamples = irArray[irToUse].getNumSamples();

    for (int i = 0; i < irSampleCounter.size(); i++) irSampleCounter[i] = 0;

    int irLength = irArray[irToUse].getNumSamples();

    numActiveSlots = numSlots;
    for (int partitionIndex = 0; partitionIndex < numSlots; partitionIndex++)
    {
        int indexAtStartOfSegment = irSampleCounter[0];

        // Generate the trimming decay curve for this segment of the impulse response.
        updateDecayCurve(decayCurve.data(), decayValue, headSize, indexAtStartOfSegment, numSamples);

        // Apply the trimming curve to the impulse response. 
        for (int channel = 0; channel < temporarySegment.getNumChannels(); channel++)
        {
            auto* irChannelData = irArray[irToUse].getReadPointer(channel) + irSampleCounter[channel];
            auto* decayCurveData = decayCurve.data();
            auto* tempSegmentData = temporarySegment.getWritePointer(channel);

            // Multiply the curve data by the irChannelData, and place it into the temporary segmentData. 
            multiplyAddSIMD(tempSegmentData, irChannelData, decayCurveData, headSize);

            irSampleCounter[channel] += headSize;
        }

        if (irSegments[partitionIndex] == NULL)
        {
            irSegments[partitionIndex] = std::make_unique<ImpulseResponseSegment>(&temporarySegment, indexAtStartOfSegment, &fft);
        }
        else
        {
            irSegments[partitionIndex]->generateComplexPartitions(&temporarySegment);
        }
        // See if you can make this more efficient????

        bool tooQuiet = juce::Decibels::gainToDecibels(decayCurve.back()) <= -90;
        bool runOutOfSamples = irSampleCounter[0] >= irLength;

        if (tooQuiet || runOutOfSamples) {
            numActiveSlots = partitionIndex;
            break;
        }
    }

    // Clear all the other slots which are not being used, and may contain junk.
    // This helps prevent artifacts when changing the decay parameter.
    for (int slotToClear = numActiveSlots; slotToClear < numSlots; slotToClear++)
    {
        for (int channel = 0; channel < numChannels; channel++)
        {
            fillWithZerosSIMD(reinterpret_cast<float*>(fdl[slotToClear][channel].data()),
                (int)fdl[slotToClear][channel].size());
        }
    }
}

void FdlConvolver::updateDecayCurve(float* __restrict decayCurve, float decayValue, int headSize, int _indexAtStartOfSegment, int _numSamples)
{
    float numSamples = static_cast<float>(_numSamples);

#pragma omp simd
    for (int i = 0; i < headSize; i++)
    {
        float rampValue = (numSamples - static_cast<float>(_indexAtStartOfSegment + i)) / numSamples;
        *decayCurve++ = powf(rampValue, decayValue);
    }
}

