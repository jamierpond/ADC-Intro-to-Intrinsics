/*
  ==============================================================================

    CounterTimerComponent.h
    Created: 7 Oct 2021 2:22:17pm
    Author:  jamie

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

struct ButtonPanel : juce::Component
{
    ButtonPanel()
    {
        startButton.setButtonText("Start");
        resetButton.setButtonText("Reset");
        stopButton.setButtonText("Stop");

        addAndMakeVisible(startButton);
        addAndMakeVisible(resetButton);
        addAndMakeVisible(stopButton);
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

        background.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
        background.items = { juce::GridItem(startButton),
                             juce::GridItem(resetButton),
                             juce::GridItem(stopButton) };
        background.performLayout(bounds);
    }

    juce::TextButton startButton, stopButton, resetButton;
};

struct TimerDisplayPanel : juce::Component, juce::Timer
{
    TimerDisplayPanel()
    {
        startTimerHz(30);
    }

    ~TimerDisplayPanel()
    {
        stopTimer();
    }

    void paint(juce::Graphics& g)
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

        g.setFont(juce::Font(16.0f));
        g.setColour(juce::Colours::white);

        float numSecondsToDisplay = (float)numMillisecondsPassed / 1000.f;

        g.drawText(juce::String(numSecondsToDisplay), getLocalBounds(), juce::Justification::centred, true);
    }

    void resized()
    {

    }

    int numMillisecondsPassed = 0;
};

struct DragRacer : juce::Component, juce::Timer
{

    DragRacer()
    {
        addAndMakeVisible(timerDisplay);
        addAndMakeVisible(buttons);

        buttons.startButton.onClick = [this]()
        {
            reset();
            startTimer(1);
        };

        buttons.stopButton.onClick = [this]()
        {
            stopTimer();
        };
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
    }

    void timerCallback() override
    {
        if (++timerDisplay.numMillisecondsPassed > numSecondsToTest * 1000)
            stopTimer();
    }

    void reset()
    {
        timerDisplay.numMillisecondsPassed = 0;
    }

    TimerDisplayPanel timerDisplay;
    ButtonPanel buttons;

    int numSecondsToTest = 10;
};