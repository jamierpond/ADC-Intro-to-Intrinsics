/***** ImpulseResponseSegment.h *****/

#pragma once
#include <vector> 
#include <complex> 
#include "pffft-master/pffft.hpp"
#include "ConvolutionUtilities.h"
#include <immintrin.h>

// FFT is defined in the file above apparently.
// #include <libraries/Fft/Fft.h>

struct ImpulseResponseSegment
{
	ImpulseResponseSegment(juce::AudioBuffer<float>* ir, int  _indexInIR, pffft::Fft<float>* _fft)
		: fft(_fft),
		indexInIR(_indexInIR),
		mFftSize(fft->getLength())
	{
		const int numChannels  = ir->getNumChannels();
		const int numIrSamples = ir->getNumSamples(); 

		zeropaddedIR.resize(mFftSize);
		interleavedSpectrum.resize(mFftSize);

		complexPartition.resize(numChannels);
		for (int channel = 0; channel < numChannels; channel++)
			complexPartition[channel].resize(mFftSize);

		generateComplexPartitions(ir); 

		isInitalized = true;
	}

	void generateComplexPartitions(juce::AudioBuffer<float>* ir)
	{
		const int numChannels = ir->getNumChannels();
		const int numIrSamples = ir->getNumSamples();

		for (int channel = 0; channel < numChannels; channel++)
		{
			auto* irData = ir->getReadPointer(channel);
			auto* zeropaddedIRData = zeropaddedIR.data();

			auto* irData_simd = reinterpret_cast<const __m256*>(irData);
			auto* zeroPaddedData_simd = reinterpret_cast<__m256*>(zeropaddedIRData);

			int numSIMDOperations = zeropaddedIR.size() / (8 * 2);

			for (int i = 0; i < numSIMDOperations; i++)
			{
				*zeroPaddedData_simd = *irData_simd;
				zeroPaddedData_simd++;
				irData_simd++;
			}

			fft->forward(zeropaddedIR.data(), interleavedSpectrum.data());
			deinterleave((complexPartition[channel].data()),
				reinterpret_cast<float*>(interleavedSpectrum.data()), mFftSize);
		}
	}

	int fftSizeToFftOrder(int fftSize)
	{
		return int(std::log2f(fftSize));
	}

	int partitionSize()
	{
		return mFftSize / 2;
	}


	pffft::Fft<float>* fft;
	std::vector<float> zeropaddedIR;
	std::vector<std::vector<float>> complexPartition;
	std::vector<Complex> interleavedSpectrum;
	int indexInIR;
	int mFftSize;

	bool isInitalized = false;
};
