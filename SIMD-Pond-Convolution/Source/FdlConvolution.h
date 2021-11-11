/*
  ==============================================================================
    FdlConvolution.h
    Created: 8 Jun 2021 1:14:47pm
    Author:  jamie
  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "ImpulseResponseSegment.h"
#include "ConvolutionUtilities.h"
#include "pffft-master/pffft.hpp"
#include "CircularBuffer.h"

#include "../../SIMD Functions/PondSIMDFunctions.h"

class FdlConvolver
{
public:

    FdlConvolver(int _sampleRate,
        int _headSize,
        CircularBuffer* _inputBuffer,
        CircularBuffer* _outputBuffer,
        std::vector<BinaryDataNameSizePair> _impulseResponsesToLoad)
        : headSize(_headSize),
        mFftSize(_headSize * 2),
        inputBuffer(_inputBuffer),
        outputBuffer(_outputBuffer),
        fft(mFftSize),
        hostSampleRate(_sampleRate),
        impulseResponsesToLoad(_impulseResponsesToLoad)
    {
        setup();
        updateImpulseResponse(1.0f, 0);
        mIsInitialized = true;
    }

    void setup();

    /* Creates a ramp, and then raises each element of the ramp to a given power, which creates a nice exponential decay.
     * This is nice for shortening the impulse responses.
     * This now uses pointer arithmetic, and a simd pragma, so hopefully it's a bit quicker than before!
     */
    void updateDecayCurve(float* __restrict decayCurve,
                          float decayValue,
                          int headSize, 
                          int _indexAtStartOfSegment, 
                          int _numSamples);

    void updateImpulseResponse(float decayValue, int _targetIr);

    void process(int cachedInputBufferPointer, 
                 int cachedOuputBufferPointer,
                 bool shouldUseExternalBuffer = false, 
                 juce::AudioBuffer<float>* _inputBuffer = nullptr);

    void setHeadSize(int _headSize)
    {
        headSize = _headSize;
        setup();
    }

    void setDelayInBlocks(int delayInBlocks) { mReadPosition = modulo(mReadPosition - delayInBlocks, (int)fdl.size()); }

    void advanceReadPosition(int _increment = 1) { mReadPosition = (mReadPosition + _increment) % numActiveSlots; }

    void advanceWritePosition(int _increment = 1) { mWritePosition = (mWritePosition + _increment) % numActiveSlots; }

    int getHeadSize()
    {
        return headSize;
    }

    bool isInitialized()
    {
        return mIsInitialized;
    }

    void setUseOptimizedFunction(bool shouldUse)
    {
        shouldUseOptimizedFunction = shouldUse;
    }

private:

    int headSize = -1;
    int mFftSize = -1;
    int numSlots = -1;
    int numActiveSlots = -1;

    int mWritePosition = 0;
    int mReadPosition = 0;

    bool mIsInitialized = false;

    // fdl is basically a circular buffer of multichannel complex blocks. 
    std::vector<MultiChannelFloatVector> fdl;
    std::vector<std::unique_ptr<ImpulseResponseSegment>> irSegments;

    CircularBuffer* inputBuffer;
    CircularBuffer* outputBuffer;

    std::vector<float> decayCurve;

    juce::AudioBuffer<float> bufferToProcess;
    juce::AudioBuffer<float> outputBufferToProcess;
    juce::AudioBuffer<float> temporarySegment;

    bool shouldUseOptimizedFunction = false;

    pffft::Fft<float> fft;

    std::vector<int> irSampleCounter;

    std::vector<juce::AudioBuffer<float>> irArray;
    int irToUse = 0;

    ComplexVector outputData;
    ComplexVector interleavedComplex;

    const int sampleRateOfImpulseResponses = 48000;
    const int hostSampleRate;

    std::vector<BinaryDataNameSizePair> impulseResponsesToLoad;

    // =============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FdlConvolver)
};
