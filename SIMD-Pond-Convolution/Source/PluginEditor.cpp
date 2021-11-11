/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SIMDPondConvolutionAudioProcessorEditor::SIMDPondConvolutionAudioProcessorEditor (SIMDPondConvolutionAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible(optimizedChoiceButton);
    optimizedChoiceButton.setButtonText("Use optimized function?");

    optimizedChoiceButton.onStateChange = [this]()
    {
        updateProcessor(optimizedChoiceButton.getToggleState());
    };

    setSize (400, 300);
}

SIMDPondConvolutionAudioProcessorEditor::~SIMDPondConvolutionAudioProcessorEditor()
{
}

//==============================================================================
void SIMDPondConvolutionAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void SIMDPondConvolutionAudioProcessorEditor::resized()
{
    optimizedChoiceButton.setBounds(getLocalBounds());
}
