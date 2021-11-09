#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    addAndMakeVisible(racer);

    setSize (600, 400);
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    
}

void MainComponent::resized()
{
    auto leftBounds = getLocalBounds().removeFromRight(getLocalBounds().getWidth() / 2);
    auto rightBounds = getLocalBounds().removeFromLeft(getLocalBounds().getWidth() / 2);

    racer.setBounds(getLocalBounds());

}