/*
  ==============================================================================

    ComplexMultiplicationFunctions.h
    Created: 11 Nov 2021 2:07:45am
    Author:  jamie

  ==============================================================================
*/

#pragma once
#include <immintrin.h>
#include "MultiplyAddFunctions.h"

void fillWithZeros(float* __restrict data, int numElements)
{
    for (int i = 0; i < numElements; i++)
    {
        *data = 0.0f;
        data++;
    }
}

void fillWithZerosSIMD(float* __restrict data, int numElements)
{
    int oddNumElementsToDo = numElements % 8;
    if (oddNumElementsToDo != 0)
    {
        fillWithZeros(data, oddNumElementsToDo);
        data += oddNumElementsToDo;
        numElements -= oddNumElementsToDo;
    }

    auto* dataSIMD = getSIMDPointer(data);

    int numSIMD = numElements / 8;

    for (int i = 0; i < numSIMD; i++)
    {
        *dataSIMD = _mm256_set1_ps(0.0f);
        dataSIMD++;
    }
}

void noOptComplexMult(float* __restrict output,
                      const float* __restrict left,
                      const float* __restrict right,
                      int numElements)
{
    int halfSize = numElements / 2;

    // Take care of the 0th bin/Nyquist thing. 
    *(output) += *(left) * *(right);
    *(output + halfSize) += *(left + halfSize) * *(right + halfSize);

    output += 1;
    left   += 1;
    right  += 1;

    auto outputReal = output;
    auto outputImag = output + halfSize;

    auto leftReal = left;
    auto leftImag = left + halfSize;

    auto rightReal = right;
    auto rightImag = right + halfSize;

    for (int i = 0; i < halfSize - 1; i++)
    {
        outputReal[i] += leftReal[i] * rightReal[i];
        outputReal[i] -= leftImag[i] * rightImag[i];

        outputImag[i] += leftReal[i] * rightImag[i];
        outputImag[i] += leftImag[i] * rightReal[i];
    }
}

void juceVectorComplexMult(float* __restrict output,
    const float* __restrict left,
    const float* __restrict right, 
    int numElements)
{
    int halfSize = numElements / 2;

    // Take care of the 0th bin/Nyquist thing. 
    *(output) += *(left) * *(right);
    *(output + halfSize) += *(left + halfSize) * *(right + halfSize);

    output += 1;
    left   += 1;
    right  += 1;

    auto outputReal = output;
    auto outputImag = output + halfSize;

    auto leftReal = left;
    auto leftImag = left + halfSize;

    auto rightReal = right;
    auto rightImag = right + halfSize;

    // Real part. 
    juce::FloatVectorOperations::addWithMultiply(outputReal, leftReal, rightReal, halfSize - 1);
    juce::FloatVectorOperations::subtractWithMultiply(outputReal, leftImag, rightImag, halfSize - 1);

    // Imag part.
    juce::FloatVectorOperations::addWithMultiply(outputImag, leftReal, rightImag, halfSize - 1);
    juce::FloatVectorOperations::addWithMultiply(outputImag, leftImag, rightReal, halfSize - 1);
}

inline void complexMultiplicationSIMD(float* __restrict output,
                                      float* __restrict left,
                                      float* __restrict right,
                                      int numElements)
{
    const int halfSize = numElements / 2;
    const int numSIMDInstructions = halfSize / 8;

    // Get the 0th bin thing done nicely.
    float outputRealNyq = *output + *(left) * *(right);
    float outputImagNyq = *(output + halfSize) + *(left + halfSize) * *(right + halfSize);

    auto* outputReal = reinterpret_cast<__m256*>(output);
    auto* outputImag = reinterpret_cast<__m256*>(output + halfSize);

    auto* leftReal = reinterpret_cast<__m256*>(left);
    auto* leftImag = reinterpret_cast<__m256*>(left + halfSize);

    auto* rightReal = reinterpret_cast<__m256*>(right);
    auto* rightImag = reinterpret_cast<__m256*>(right + halfSize);

    for (int i = 0; i < numSIMDInstructions; i++)
    {
        *outputReal = _mm256_add_ps(*outputReal, _mm256_mul_ps(*leftReal, *rightReal));
        *outputReal = _mm256_sub_ps(*outputReal, _mm256_mul_ps(*leftImag, *rightImag));

        *outputImag = _mm256_add_ps(*outputImag, _mm256_mul_ps(*leftReal, *rightImag));
        *outputImag = _mm256_add_ps(*outputImag, _mm256_mul_ps(*leftImag, *rightReal));

        outputReal++;
        outputImag++;
        leftReal++;
        rightReal++;
        leftImag++;
        rightImag++; 
    }

    // Take care of the 0th bin/Nyquist thing.
    *(output) = outputRealNyq;
    *(output + halfSize) = outputImagNyq;
}

void complexMultiplication(float* __restrict output,
    float* __restrict left,
    float* __restrict right,
    int numElements,
    bool useOptimizedCode = false)
{

    // noOptComplexMult(output, left, right, numElements);

    if (useOptimizedCode)
        complexMultiplicationSIMD(output, left, right, numElements);
    else
    {
        noOptComplexMult(output, left, right, numElements);
    }
}
