#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{

    addAndMakeVisible(racerNormal);
    addAndMakeVisible(racerOptimized);

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

    racerNormal.setBounds(leftBounds);
    racerOptimized.setBounds(rightBounds);

}
