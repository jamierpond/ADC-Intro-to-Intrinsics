/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class SIMDPondConvolutionAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SIMDPondConvolutionAudioProcessorEditor (SIMDPondConvolutionAudioProcessor&);
    ~SIMDPondConvolutionAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SIMDPondConvolutionAudioProcessor& audioProcessor;

    void updateProcessor(bool shouldBeOptimized)
    {
        audioProcessor.setShouldBeOptimized(shouldBeOptimized);
    }

    juce::ToggleButton optimizedChoiceButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SIMDPondConvolutionAudioProcessorEditor)
};
