/*
  ==============================================================================

    ComplexMultiplicationFunctions.h
    Created: 11 Nov 2021 2:07:45pm
    Author:  jamie

  ==============================================================================
*/

#pragma once
#include "SIMDHelpers.h"
#include "JuceHeader.h"
// Complex multiplication:

inline void noOptComplexMult(float* output,
    const float* left,
    const float* right,
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

inline void juceVectorComplexMult(float* __restrict output,
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
                                      const float*  __restrict left,
                                      const float* const __restrict right,
                                      int numElements)
{
    const int halfSize = numElements / 2;

    // We have to do this first because for speed we'll overwrite these values and add them on later. 
    float outputRealNyq = output[0] + left[0] * right[0];
    float outputImagNyq = output[0 + halfSize] + left[0 + halfSize] * right[0 + halfSize];

    auto* outputReal = getSIMDPointer(output);
    auto* outputImag = getSIMDPointer(output + halfSize);

    auto* leftReal   = getConstSIMDPointer(left);
    auto* leftImag   = getConstSIMDPointer(left + halfSize);

    auto* rightReal  = getConstSIMDPointer(right);
    auto* rightImag  = getConstSIMDPointer(right + halfSize);

    const int numSIMD = halfSize / 8;

    for (int i = 0; i < numSIMD; i++)
    {
        outputReal[i] = _mm256_fmadd_ps(leftReal[i], rightReal[i], outputReal[i]);
        outputReal[i] = _mm256_sub_ps(outputReal[i], _mm256_mul_ps(leftImag[i], rightImag[i]));

        outputImag[i] = _mm256_fmadd_ps(leftReal[i], rightImag[i], outputImag[i]);
        outputImag[i] = _mm256_fmadd_ps(leftImag[i], rightReal[i], outputImag[i]);
    }

    // Take care of the 0th bin/Nyquist thing from earlier. 
    output[0] = outputRealNyq;
    output[halfSize] = outputImagNyq;
}

inline void complexMultiplication(float* __restrict output,
                                  const float* __restrict left,
                                  const float*__restrict right,
                                  int numElements,
                                  bool useOptimizedCode)
{
    if (useOptimizedCode)
        complexMultiplicationSIMD(output, left, right, numElements);
    else
    {
        juceVectorComplexMult(output, left, right, numElements);
    }
}