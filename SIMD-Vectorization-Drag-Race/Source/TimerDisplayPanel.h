/*
  ==============================================================================

    TimerDisplayPanel.hg.h
    Created: 9 Nov 2021 7:49:38pm
    Author:  Jamie

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

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