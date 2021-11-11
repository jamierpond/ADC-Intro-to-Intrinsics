/***** CircularBuffer.h *****/

#pragma once
#include <vector>
#include "JuceHeader.h"
#include "ConvolutionUtilities.h"

/*
  ==============================================================================

	CircularBuffer.h
	Created: 3 Jun 2021 3:07:20pm
	Author:  Jamie

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

class CircularBuffer
{
public:
	CircularBuffer(int numChannels, int numSamples) : buffer(numChannels, numSamples), circularBufferLength(numSamples)
	{
		for (int i = 0; i < numChannels; ++i)
			buffer.clear(i, 0, buffer.getNumSamples());
	}

	void copyToCircularBuffer(int channel, const int bufferToReadLength, const float* bufferToReadData, int givenReadPointer = -1)
	{
		int writePositionToUse = modulo((givenReadPointer == -1) ? mWritePosition : givenReadPointer, circularBufferLength);

		if (buffer.getNumSamples() > bufferToReadLength + writePositionToUse)
		{
			buffer.copyFrom(channel, writePositionToUse, bufferToReadData, bufferToReadLength);
		}
		else
		{
			const int bufferRemaining = circularBufferLength - writePositionToUse;
			buffer.copyFrom(channel, writePositionToUse, bufferToReadData, bufferRemaining);
			buffer.copyFrom(channel, 0, bufferToReadData + bufferRemaining, bufferToReadLength - bufferRemaining);
		}
	}

	void addToCircularBuffer(int channel, const int bufferToReadLength, const float* bufferToReadData, int givenReadPointer = -1)
	{
		int writePositionToUse = modulo((givenReadPointer == -1) ? mWritePosition : givenReadPointer, circularBufferLength);

		if (buffer.getNumSamples() > bufferToReadLength + writePositionToUse)
		{
			buffer.addFrom(channel, writePositionToUse, bufferToReadData, bufferToReadLength);
		}
		else
		{
			const int bufferRemaining = circularBufferLength - writePositionToUse;
			buffer.addFrom(channel, writePositionToUse, bufferToReadData, bufferRemaining);
			buffer.addFrom(channel, 0, bufferToReadData + bufferRemaining, bufferToReadLength - bufferRemaining);
		}
	}

	void getFromCircularBuffer(int channel,
		const int bufferToWriteToLength, 
		juce::AudioBuffer<float>& bufferToWriteTo, 
		bool zeroPadHalf = false,
		int givenReadPointer = -1,
	    bool clearOnRead = false)
	{

		int readPositionToUse = (givenReadPointer == -1) ? mReadPosition : givenReadPointer;
		int numSamplesToGet = bufferToWriteToLength;
		if (zeroPadHalf)
			numSamplesToGet /= 2;

		readPositionToUse = modulo(readPositionToUse - numSamplesToGet, circularBufferLength);

		auto* circularBufferData = buffer.getReadPointer(channel);

		if (circularBufferLength > numSamplesToGet + readPositionToUse)
		{
			bufferToWriteTo.addFrom(channel, 0, circularBufferData + readPositionToUse, numSamplesToGet);

			if(clearOnRead)
				buffer.clear(channel, readPositionToUse, numSamplesToGet);
		}
		else
		{
			const int bufferRemaining = circularBufferLength - readPositionToUse;
			const int bufferToReadAtStart = circularBufferLength - bufferRemaining;
			bufferToWriteTo.addFrom(channel, 0, circularBufferData + readPositionToUse, bufferRemaining);
			bufferToWriteTo.addFrom(channel, bufferRemaining, circularBufferData, numSamplesToGet - bufferRemaining);

			if (clearOnRead)
			{
				buffer.clear(channel, readPositionToUse, bufferRemaining);
				buffer.clear(channel, 0, numSamplesToGet - bufferRemaining);
			}
		}
	}

	void setDelayInSamples(int delay)
	{
		// mReadPosition = (circularBufferLength + mWritePosition - delay) % circularBufferLength;
		mReadPosition = modulo(mWritePosition - delay, circularBufferLength);
	}

	void advanceReadPosition(int increment = 1)
	{
		mReadPosition += increment + circularBufferLength;
		mReadPosition %= circularBufferLength;
	}

	void advanceWritePosition(int increment = 1)
	{
		mWritePosition += increment + circularBufferLength;
		mWritePosition %= circularBufferLength;
	}

	int getLength()
	{
		return circularBufferLength;
	}

	int getReadPosition()
	{
		return mReadPosition;
	}

	int getWritePosition()
	{
		return mWritePosition;
	}

private:

	juce::AudioBuffer<float> buffer;
	int mWritePosition = 0;
	int mReadPosition = 0;
	int circularBufferLength;
};