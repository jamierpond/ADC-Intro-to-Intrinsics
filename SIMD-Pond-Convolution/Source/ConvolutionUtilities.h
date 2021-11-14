/*
  ==============================================================================
    ConvolutionUtilities.h
    Created: 8 Jun 2021 1:26:35pm
    Author:  jamie
  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

typedef std::complex<float> Complex;
typedef std::vector<Complex> ComplexVector;
typedef std::vector<ComplexVector> MultiChannelComplexVector;

typedef std::vector<float> FloatVector;
typedef std::vector<FloatVector> MultiChannelFloatVector;

class Time
{
public:
    Time() {}

    void setSampleRate(float _fs)
    {
        fs = _fs;
    }

    int secondsToSamples(float _seconds)
    {
        return _seconds * fs;
    }

private:
    float fs = 44100.f;
};

class MultiChannelIIRFilter
{
public:
    void setup(int numChannels)
    {
        filters.resize(numChannels);
        reset();
    }

    void reset()
    {
        for (auto& f : filters){ f.reset(); }
    }

    void processSamples(int channel, float* samplesToProcess, const int numSamplesToProcess)
    {
        filters[channel].processSamples(samplesToProcess, numSamplesToProcess);
    }
    
    void setCoefficients(juce::IIRCoefficients coeffs)
    {
        for (auto& f : filters) { f.setCoefficients(coeffs); }
    }

private:
    std::vector<juce::IIRFilter> filters;
};

struct BinaryDataNameSizePair
{
    BinaryDataNameSizePair(const char* _name, const int _size) : name(_name), size(_size) {}
    const char* name;
    const int size;
};

/*Assumes no x smaller than -upperValue*/
inline int modulo(int x, int upperValue)
{
    return (x >= upperValue || x < 0) ? (x + upperValue) % upperValue : x;
}

template <class T>
inline bool AreSame(T a, T b)
{
    return fabs(a - b) < std::numeric_limits<T>::epsilon();
}

inline void interleave(float* __restrict interleavedOutput, const float* __restrict deinterleavedInput, int size)
{
    int halfSize = size / 2;

    float* intlEven = interleavedOutput;
    float* intlOdd = interleavedOutput + 1;

    const float* deintBottom = deinterleavedInput;
    const float* deintTop    = deinterleavedInput + halfSize;

    for (int n = 0; n < halfSize; n++)
    {
        *intlEven = *deintBottom;
        *intlOdd  = *deintTop;

        intlEven += 2;
        intlOdd  += 2;

        deintBottom += 1;
        deintTop    += 1;
    }
}

inline void deinterleave(float* __restrict deinterleavedOutput, const float* __restrict interleavedInput, int size)
{

    int halfSize = size / 2;

    const float* intlEven = interleavedInput;
    const float* intlOdd = interleavedInput + 1;

    float* deintBottom = deinterleavedOutput;
    float* deintTop = deinterleavedOutput + halfSize;

    for (int n = 0; n < halfSize; n++)
    {
        *deintBottom = *intlEven;
        *deintTop    = *intlOdd;

        intlEven += 2;
        intlOdd  += 2;

        deintBottom += 1;
        deintTop    += 1;
    }
}

inline juce::AudioBuffer<float> resampleBuffer(const juce::AudioBuffer<float>& oldSampleRateBuffer, double inrate, double outrate)
{
    juce::AudioBuffer<float> newSampleRateBuffer;
    newSampleRateBuffer.clear();

    double ratio = (double)inrate / (double)outrate;

    newSampleRateBuffer.setSize((int)oldSampleRateBuffer.getNumChannels(),
        (int)(((double)oldSampleRateBuffer.getNumSamples()) / ratio));

    std::unique_ptr<juce::LagrangeInterpolator> resampler = std::make_unique<juce::LagrangeInterpolator>();

    const float** inputs = oldSampleRateBuffer.getArrayOfReadPointers();
    float** outputs = newSampleRateBuffer.getArrayOfWritePointers();
    for (int channel = 0; channel < newSampleRateBuffer.getNumChannels(); channel++)
    {
        resampler->reset();
        resampler->process(ratio, inputs[channel], outputs[channel], newSampleRateBuffer.getNumSamples());
    }
    return newSampleRateBuffer;
}

inline void normalizeBuffer(juce::AudioBuffer<float>& buffer)
{ 
    buffer.applyGain(1.0f / buffer.getMagnitude(0, buffer.getNumSamples()));
}

inline float getPowerGainFromNumActiveSlots(int  numActiveSlots, int numTotalSlots, float power = 1.4f)
{   
    return (float)numTotalSlots * tanh(1.0f / numTotalSlots * (float)numActiveSlots);
}

inline void trimTrailingSilence(juce::AudioBuffer<float>& buffer, float silenceThesholdDB = -100.0f)
{
    float silenceThresholdGain = juce::Decibels::decibelsToGain(silenceThesholdDB);
    int indexOfSilenceStart = 0;
    for (int channel = 0; channel < buffer.getNumChannels(); channel++)
    {
        auto channelData = buffer.getReadPointer(channel);
        for (int n = (buffer.getNumSamples() - 1); n >= 0 ; n--)
        {
            if (channelData[n] > silenceThresholdGain)
            {
                indexOfSilenceStart = n; 
                break;
            }
        }
    }

    buffer.setSize(buffer.getNumChannels(), indexOfSilenceStart, true, true, false);
}


inline juce::AudioBuffer<float> wavFileLoader(const char* binaryName, const int binarySize)
{
    juce::WavAudioFormat wavFormat;
    juce::AudioBuffer<float> buffer;
    auto* inputStream = new juce::MemoryInputStream(binaryName, binarySize, false);
    std::unique_ptr<juce::AudioFormatReader> audioFormatReader(wavFormat.createReaderFor(inputStream, true));

    if (!audioFormatReader)
        return {};

    buffer.setSize(audioFormatReader->numChannels, (int)audioFormatReader->lengthInSamples);
    audioFormatReader->read(&buffer, 0, buffer.getNumSamples(), 0, true, true);

    return buffer;
} 


struct SimpleVolumeDucker
{
    SimpleVolumeDucker(float numMsForDucking, float fs)
    {
        setDuckingPeriod(numMsForDucking, fs);
    }

    void setDuckingPeriod(float numMsForDucking, float fs)
    {
        numSampesInDuckPeriod = static_cast<int>(numMsForDucking * 0.001 * fs);
    }

    void startDuck()
    {

    }

   /* float getGainValueToUse()
    {
        //  pick up from here. You need to create a ducking/crossfade so that there are no clicks when changing presets. 
    }*/

    int numSamplesProcessed = 0;

    int numSampesInDuckPeriod = 0;

};
