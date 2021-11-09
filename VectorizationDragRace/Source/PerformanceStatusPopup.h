/*
  ==============================================================================

    PerformanceStatusPopup.h
    Created: 9 Nov 2021 7:50:38pm
    Author:  Jamie

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

struct PerformaceStatusComponent : juce::Component
{
    void paint(juce::Graphics& g) override
    {
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

        g.setFont(juce::Font(16.0f));
        g.setColour(juce::Colours::white);

        juce::String doof = juce::String(numIterations + " number of iterations were completed in ") + juce::String(numSecondsCompletedIn + " seconds. ");

        g.drawText(doof, getLocalBounds(), juce::Justification::centred, true);
    }

    juce::String numIterations, numSecondsCompletedIn;
};