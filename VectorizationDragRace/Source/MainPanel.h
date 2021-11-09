/*
  ==============================================================================

    CounterTimerComponent.h
    Created: 7 Oct 2021 2:22:17pm
    Author:  jamie

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

#include "ControlPanel.h"
#include "CodeRunnerThread.h"
#include "PerformanceStatusPopup.h"
#include "TimerDisplayPanel.h"


struct MainPanel : juce::Component, juce::Timer
{
    MainPanel()
        : timerDisplay(numMsSecondsPassed), 
        notificationPopup("Performance Notification", juce::Colours::darkgrey, true)
    {

        codeRunnerThread.resizeBuffers(int(powf(2, 5 + controlPanel.bufferSizeChoice.getSelectedId())));
        codeRunnerThread.setNumIterationsToDo(controlPanel.numIterationsSlider.getValue());

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