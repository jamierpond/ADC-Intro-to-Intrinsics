/*
  ==============================================================================

    CounterTimerComponent.h
    Created: 7 Oct 2021 2:22:17pm
    Author:  jamie

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"


enum AlgoChoice
{
    NoOptimization = 1,
    JuceFloatVector,
    HandCodedSIMD,
};


struct ButtonPanel : juce::Component
{
    ButtonPanel()
    {
        startButton.setButtonText("Start");
        resetButton.setButtonText("Reset");
        stopButton.setButtonText("Stop");

        algoChoice.addItem("No optimization", 1);
        algoChoice.addItem("juce::FloatVectorOperations", 2);
        algoChoice.addItem("Hand coded x86/x64 SIMD", 3);
        algoChoice.setSelectedId(1, false);

        addAndMakeVisible(startButton);
        addAndMakeVisible(resetButton);
        addAndMakeVisible(stopButton);
        addAndMakeVisible(algoChoice);
    }

    void paint(juce::Graphics& g)
    {

    }

    void resized()
    {
        auto bounds = getLocalBounds();

        // shorthands for grid class
        using Track = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;

        // grid
        juce::Grid background;
        background.templateRows = { Track(Fr(1)) };

        background.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
        background.items = { juce::GridItem(startButton),
                             juce::GridItem(resetButton),
                             juce::GridItem(stopButton),
                             juce::GridItem(algoChoice) };
        background.performLayout(bounds);
    }

    juce::TextButton startButton, stopButton, resetButton;
    juce::ComboBox algoChoice;
};

struct TimerDisplayPanel : juce::Component
{
    TimerDisplayPanel(float& numMsPassed)
        : numMillisecondsPassed(numMsPassed)
    {
    }

    ~TimerDisplayPanel()
    {
    }

    void paint(juce::Graphics& g)
    {
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

        g.setFont(juce::Font(16.0f));
        g.setColour(juce::Colours::white);

        float numSecondsToDisplay = (float)numMillisecondsPassed / 1000.f;

        g.drawText(juce::String(numSecondsToDisplay), getLocalBounds(), juce::Justification::centred, true);
    }

    void resized()
    {

    }

    float& numMillisecondsPassed;
};

struct CodeRunnerThread : juce::Thread
{
    CodeRunnerThread() : juce::Thread("CodeRunnerThread")
    {
        someRandomData.resize(bufferSize);
        someMoreRandomData.resize(bufferSize);
        output.resize(bufferSize);
    }

    void setAlgorithmChoice(int algToSet)
    {
        algoChoice = algToSet;
    }
    

    void run() override
    {
        numIterationsDone = 0;

        shouldStop = false;
 
        while (!shouldStop)
        {
            if (algoChoice == HandCodedSIMD)
                complex_vector_hand_mul_deinterleaved(output.data(), someRandomData.data(), someMoreRandomData.data(), bufferSize);
            else if (algoChoice == JuceFloatVector)
                NOTOPTIMIZEDcomplex_vector_hand_mul_deinterleaved(output.data(), someRandomData.data(), someMoreRandomData.data(), bufferSize);
            else if (algoChoice == NoOptimization)
                TOTALLYNOTOPTIMIZEDcomplex_vector_hand_mul_deinterleaved(output.data(), someRandomData.data(), someMoreRandomData.data(), bufferSize);
            else {}
            numIterationsDone++;
        }
    }

    float outputRealNyq;
    float outputImagNyq;
    inline void complex_vector_hand_mul_deinterleaved(float* __restrict output,
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

    inline void TOTALLYNOTOPTIMIZEDcomplex_vector_hand_mul_deinterleaved(float* __restrict output,
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


    inline void NOTOPTIMIZEDcomplex_vector_hand_mul_deinterleaved(float* __restrict output,
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

    int bufferSize = 2048;
    
    std::atomic<bool> shouldStop{ false };
    bool shouldRunOptimized = true;
    size_t numIterationsDone = 0;

    int algoChoice = AlgoChoice::NoOptimization;

    std::vector<float> someRandomData, someMoreRandomData, output;
};

struct PerformaceStatusComponent : juce::Component
{
    void paint(juce::Graphics& g) override
    {
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

        g.setFont(juce::Font(16.0f));
        g.setColour(juce::Colours::white);

        juce::String doof = juce::String(numIterations + " number of iterations were completed in ")  + juce::String(numSecondsCompletedIn + " seconds. ");

        g.drawText(doof, getLocalBounds(), juce::Justification::centred, true);
    }

    juce::String numIterations, numSecondsCompletedIn;
};

struct DragRacer : juce::Component, juce::Timer
{
    DragRacer()
        : timerDisplay(numMsSecondsPassed), 
        notificationPopup("Performance Notification", juce::Colours::darkgrey, true)
    {
        addAndMakeVisible(timerDisplay);
        addAndMakeVisible(buttons);

        buttons.startButton.onClick = [this]()
        {
            reset();
            startTimerHz(30);
            codeRunnerThread.startThread(9);
        };

        buttons.algoChoice.onChange = [this]()
        {
            codeRunnerThread.setAlgorithmChoice(buttons.algoChoice.getSelectedIdAsValue().getValue());
        };

        buttons.stopButton.onClick = [this]()
        {
            stop();
        };
    }

    void stop()
    {
        stopTimer();
        codeRunnerThread.signalToStop();
        timerDisplay.repaint();
        auto numIterationsStr = juce::String(codeRunnerThread.getNumIterationsDone());
        performanceStatus.numIterations = numIterationsStr;
        juce::SystemClipboard::copyTextToClipboard(numIterationsStr);
        performanceStatus.numSecondsCompletedIn = juce::String(numMsSecondsPassed / 1000.0f);
        notificationPopup.showDialog("Performance Overview", &performanceStatus, this, juce::Colours::black, true);
    }

    void paint(juce::Graphics& g)
    {

    }

    void resized()
    {
        auto bounds = getLocalBounds();

        auto buttonBounds = bounds.removeFromBottom(bounds.getHeight() * 0.1);
        buttons.setBounds(buttonBounds);

        timerDisplay.setBounds(bounds);
        performanceStatus.setBounds(getLocalBounds());
    }

    void timerCallback() override
    {
        numMsSecondsPassed += getTimerInterval();
        if (numMsSecondsPassed > numSecondsToTest * 1000.0f)
        {
            stop();
        }

        repaint();       
    }

    void reset()
    {
        timerDisplay.numMillisecondsPassed = 0;
    }

    TimerDisplayPanel timerDisplay;

    ButtonPanel buttons;

    CodeRunnerThread codeRunnerThread;

    PerformaceStatusComponent performanceStatus;

    juce::DialogWindow notificationPopup;

    float numMsSecondsPassed = 0.0f;

    int numSecondsToTest = 10;
};