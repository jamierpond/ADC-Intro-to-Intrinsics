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
#include "../../SIMD Functions/PondSIMDFunctions.h"

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

        float tempA, tempB;

        while (numIterationsToDo-- > 0)
        {
            if (threadShouldExit())
                return;

            if (algoChoice == NoOptimization)
                noOptComplexMult(output.data(), someRandomData.data(), someMoreRandomData.data(), bufferSize);
            else if (algoChoice == JuceFloatVector)
                juceVectorComplexMult(output.data(), someRandomData.data(), someMoreRandomData.data(), bufferSize);
            else if (algoChoice == HandCodedSIMD)
                complexMultiplicationSIMD(output.data(), someRandomData.data(), someMoreRandomData.data(), bufferSize, tempA, tempB);
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

