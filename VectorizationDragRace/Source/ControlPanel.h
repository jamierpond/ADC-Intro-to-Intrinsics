/*
  ==============================================================================

    ControlPanel.h
    Created: 9 Nov 2021 7:49:23pm
    Author:  Jamie

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

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
        bufferSizeChoice.setSelectedId(1, false);

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