/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SIMDPondConvolutionAudioProcessor::SIMDPondConvolutionAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

SIMDPondConvolutionAudioProcessor::~SIMDPondConvolutionAudioProcessor()
{
}

//==============================================================================
const juce::String SIMDPondConvolutionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SIMDPondConvolutionAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SIMDPondConvolutionAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SIMDPondConvolutionAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SIMDPondConvolutionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SIMDPondConvolutionAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SIMDPondConvolutionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SIMDPondConvolutionAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SIMDPondConvolutionAudioProcessor::getProgramName (int index)
{
    return {};
}

void SIMDPondConvolutionAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SIMDPondConvolutionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    std::vector<BinaryDataNameSizePair> irsToLoad;
    irsToLoad.push_back(BinaryDataNameSizePair(BinaryData::superlong_ir_wav, BinaryData::superlong_ir_wavSize));
    fdlConvolver = std::make_unique<FdlConvolver>(sampleRate, samplesPerBlock, &inputBuffer, &outputBuffer, irsToLoad);
}

void SIMDPondConvolutionAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SIMDPondConvolutionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SIMDPondConvolutionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // This is not yet suited for variable buffer sizes!
    int bufferSize = buffer.getNumSamples();
    if (bufferSize != fdlConvolver->getHeadSize())
    {
        jassertfalse;
        return;
    }

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Copy the input samples into the input buffer. 
    for (int channel = 0; channel < buffer.getNumChannels(); channel++)
    {
        auto* bufferData = buffer.getReadPointer(channel);
        inputBuffer.copyToCircularBuffer(channel, bufferSize, bufferData, inputBuffer.getWritePosition());
    }

    inputBuffer.advanceWritePosition(bufferSize);
    inputBuffer.advanceReadPosition(bufferSize);

    fdlConvolver->process(inputBuffer.getReadPosition(), outputBuffer.getWritePosition());

    outputBuffer.advanceWritePosition(bufferSize);

    buffer.clear();

    // Copy the output buffer to the buffer to pass back to the DAW. 
    for (int channel = 0; channel < buffer.getNumChannels(); channel++)
    {
        outputBuffer.getFromCircularBuffer(channel, buffer.getNumSamples(), buffer, false, outputBuffer.getReadPosition(), true);
    }

    outputBuffer.advanceReadPosition(bufferSize);

}

//==============================================================================
bool SIMDPondConvolutionAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SIMDPondConvolutionAudioProcessor::createEditor()
{
    return new SIMDPondConvolutionAudioProcessorEditor (*this);
}

//==============================================================================
void SIMDPondConvolutionAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SIMDPondConvolutionAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SIMDPondConvolutionAudioProcessor();
}
