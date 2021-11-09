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

struct ControlPanel : juce::Component
{
    ControlPanel() : numIterationsSlider(juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextBoxBelow)
    {
        startButton.setButtonText("Start");
        resetButton.setButtonText("Reset");
        stopButton.setButtonText("Stop");

        algoChoice.addItem("No optimization", 1);
        algoChoice.addItem("juce::FloatVectorOperations", 2);
        algoChoice.addItem("Hand coded x86/x64 SIMD", 3);
        algoChoice.setSelectedId(1, false);


        int oneMillion = 1000000;
        numIterationsSlider.setRange(oneMillion, 100000000000, oneMillion);
        numIterationsSlider.setNumDecimalPlacesToDisplay(0);
        

        bufferSizeChoice.addItem("64", 1);
        bufferSizeChoice.addItem("128", 2);
        bufferSizeChoice.addItem("256", 3);
        bufferSizeChoice.addItem("512", 4);
        bufferSizeChoice.addItem("1024", 5);
        bufferSizeChoice.addItem("2048", 6);
        bufferSizeChoice.addItem("4096", 7);
        bufferSizeChoice.addItem("8192", 8);


        addAndMakeVisible(startButton);
        addAndMakeVisible(stopButton);
        addAndMakeVisible(algoChoice);
        addAndMakeVisible(numIterationsSlider);
        addAndMakeVisible(bufferSizeChoice);
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

        background.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
        background.items = { juce::GridItem(startButton), 
                             juce::GridItem(stopButton),
                             juce::GridItem(algoChoice),
                             juce::GridItem(numIterationsSlider),
                             juce::GridItem(bufferSizeChoice) };
        background.performLayout(bounds);
    }

    juce::TextButton startButton, stopButton, resetButton;
    juce::ComboBox algoChoice, bufferSizeChoice;
    juce::Slider numIterationsSlider;
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
        addAndMakeVisible(controlPanel);

        controlPanel.startButton.onClick = [this]()
        {
            reset();
            startTimerHz(30);
            codeRunnerThread.prepareForRun();
            codeRunnerThread.startThread(juce::Thread::realtimeAudioPriority);
        };

        controlPanel.algoChoice.onChange = [this]()
        {
            codeRunnerThread.setAlgorithmChoice(controlPanel.algoChoice.getSelectedIdAsValue().getValue());
        };

        controlPanel.bufferSizeChoice.onChange = [this]()
        {
            int newBufferSize = int(powf(2, 5 + (int)controlPanel.bufferSizeChoice.getSelectedIdAsValue().getValue()));
            DBG(newBufferSize);
            codeRunnerThread.resizeBuffers(newBufferSize);
        };

        controlPanel.numIterationsSlider.onValueChange = [this]()
        {
            codeRunnerThread.setNumIterationsToDo(controlPanel.numIterationsSlider.getValue());
        };

        controlPanel.stopButton.onClick = [this]()
        {
            stop();
        };
    }

    void stop()
    {
        stopTimer();
        timerDisplay.repaint();
        performanceStatus.numSecondsCompletedIn = juce::String(numMsSecondsPassed / 1000.0f);
        performanceStatus.numIterations = juce::String(codeRunnerThread.getTargetNumIterations());
        notificationPopup.showDialog("Performance Overview", &performanceStatus, this, juce::Colours::black, true);
    }

    void paint(juce::Graphics& g)
    {

    }

    void resized()
    {
        auto bounds = getLocalBounds();

        auto buttonBounds = bounds.removeFromBottom(bounds.getHeight() * 0.1);
        controlPanel.setBounds(buttonBounds);

        timerDisplay.setBounds(bounds);
        performanceStatus.setBounds(getLocalBounds());
    }

    void timerCallback() override
    {
        numMsSecondsPassed += getTimerInterval();
        if (codeRunnerThread.hasCompletedTask)
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

    ControlPanel controlPanel;

    CodeRunnerThread codeRunnerThread;

    PerformaceStatusComponent performanceStatus;

    juce::DialogWindow notificationPopup;

    float numMsSecondsPassed = 0.0f;

    int numSecondsToTest = 10;
};