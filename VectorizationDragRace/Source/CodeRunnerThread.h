/*
  ==============================================================================

    CodeRunnerThread.h
    Created: 9 Nov 2021 7:49:56pm
    Author:  Jamie

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "immintrin.h"


enum AlgoChoice
{
    NoOptimization = 1,
    JuceFloatVector,
    HandCodedSIMD,
};

struct CodeRunnerThread : juce::Thread
{
    CodeRunnerThread() : juce::Thread("CodeRunnerThread")
    {
        resizeBuffers(bufferSize);
    }

    void setAlgorithmChoice(int algToSet)
    {
        algoChoice = algToSet;
    }

    void prepareForRun()
    {
        hasCompletedTask = false;
    }

    void run() override
    {
        size_t numIterationsToDo = targetNumIterations;

        while (numIterationsToDo-- > 0)
        {
            if (threadShouldExit())
                return;

            if (algoChoice == NoOptimization)
                noOptComplexMult(output.data(), someRandomData.data(), someMoreRandomData.data(), bufferSize);
            else if (algoChoice == JuceFloatVector)
                juceVectorComplexMult(output.data(), someRandomData.data(), someMoreRandomData.data(), bufferSize);
            else if (algoChoice == HandCodedSIMD)
                simdComplexMult(output.data(), someRandomData.data(), someMoreRandomData.data(), bufferSize);
            else {}
        }

        hasCompletedTask = true;
    }

    void setNumIterationsToDo(int todo)
    {
        targetNumIterations = todo;
    }

    void resizeBuffers(int newBufferSize)
    {
        someRandomData.resize(newBufferSize);
        someMoreRandomData.resize(newBufferSize);
        output.resize(newBufferSize);
    }

    int getTargetNumIterations()
    {
        return targetNumIterations;
    }

    float outputRealNyq;
    float outputImagNyq;
    inline void simdComplexMult(float* __restrict output,
        float* __restrict left,
        float* __restrict right,
        int numElements)
    {
        const int halfSize = numElements / 2;
        const int numSIMDInstructions = halfSize / 8;

        // Get the 0th bin thing done nicely.
        outputRealNyq = *output + *(left) * *(right);
        outputImagNyq = *(output + halfSize) + *(left + halfSize) * *(right + halfSize);

        auto* outputReal = reinterpret_cast<__m256*>(output);
        auto* outputImag = reinterpret_cast<__m256*>(output + halfSize);

        auto* leftReal = reinterpret_cast<__m256*>(left);
        auto* leftImag = reinterpret_cast<__m256*>(left + halfSize);

        auto* rightReal = reinterpret_cast<__m256*>(right);
        auto* rightImag = reinterpret_cast<__m256*>(right + halfSize);

        for (int i = 0; i < numSIMDInstructions; i++)
        {
            *outputReal = _mm256_add_ps(*outputReal, _mm256_mul_ps(*leftReal, *rightReal));
            *outputReal = _mm256_sub_ps(*outputReal, _mm256_mul_ps(*leftImag, *rightImag));

            *outputImag = _mm256_add_ps(*outputImag, _mm256_mul_ps(*leftReal, *rightImag));
            *outputImag = _mm256_add_ps(*outputImag, _mm256_mul_ps(*leftImag, *rightReal));

            outputReal++;
            outputImag++;
            leftReal++;
            rightReal++;
            leftImag++;
            rightImag++;
        }

        // Take care of the 0th bin/Nyquist thing.
        *(output) = outputRealNyq;
        *(output + halfSize) = outputImagNyq;
    }

    inline void noOptComplexMult(float* __restrict output,
        const float* __restrict left,
        const float* __restrict right,
        int numElements)
    {
        int halfSize = numElements / 2;

        // Take care of the 0th bin/Nyquist thing. 
        output[0] = left[0] * right[0];
        output[halfSize] = left[halfSize] * right[halfSize];

        for (int i = 1; i < numElements / 2; i++)
        {
            int realIndex = i;
            int imagIndex = i + halfSize;

            output[realIndex] += left[realIndex] * right[realIndex];
            output[realIndex] -= left[imagIndex] * right[imagIndex];

            output[imagIndex] += left[realIndex] * right[imagIndex];
            output[imagIndex] += left[imagIndex] * right[realIndex];
        }
    }


    inline void juceVectorComplexMult(float* __restrict output,
        const float* __restrict left,
        const float* __restrict right,
        int numElements)
    {
        int halfSize = numElements / 2;

        // Take care of the 0th bin/Nyquist thing. 
        *(output) += *(left) * *(right);
        *(output + halfSize) += *(left + halfSize) * *(right + halfSize);

        output += 1;
        left += 1;
        right += 1;

        auto outputReal = output;
        auto outputImag = output + halfSize;

        auto leftReal = left;
        auto leftImag = left + halfSize;

        auto rightReal = right;
        auto rightImag = right + halfSize;

        // Real part. 
        juce::FloatVectorOperations::addWithMultiply(outputReal, leftReal, rightReal, halfSize - 1);
        juce::FloatVectorOperations::subtractWithMultiply(outputReal, leftImag, rightImag, halfSize - 1);

        // Imag part.
        juce::FloatVectorOperations::addWithMultiply(outputImag, leftReal, rightImag, halfSize - 1);
        juce::FloatVectorOperations::addWithMultiply(outputImag, leftImag, rightReal, halfSize - 1);
    }

    void signalToStop()
    {
        shouldStop = true;
    }

    int getNumIterationsDone()
    {
        return numIterationsDone;
    }

    int bufferSize = 4096;

    std::atomic<bool> shouldStop{ false };
    bool shouldRunOptimized = true;
    size_t numIterationsDone = 0;

    bool hasCompletedTask = true;

    size_t targetNumIterations = 100000;

    int algoChoice = AlgoChoice::NoOptimization;

    std::vector<float> someRandomData, someMoreRandomData, output;
};

