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

__forceinline void complexMultiplicationSIMD(float* __restrict output,
                                             const float* __restrict left,
                                             const float* __restrict right,
                                             int numElements,
                                             float& tempFloatA, float& tempFloatB)
{
    const int halfSize = numElements / 2;

    // We have to do this first because for speed we'll overwrite these values and add them on later. 
    tempFloatA = output[0] + left[0] * right[0];
    tempFloatB = output[0 + halfSize] + left[0 + halfSize] * right[0 + halfSize];
    float* _output = output;

    const int numSIMD = halfSize / 8;

    register __m256 outRe, outIm, leftRe, leftIm, rightRe, rightIm;

    for (int i = 0; i < numSIMD; i++)
    {
        // Load values from memory
        outRe = _mm256_load_ps(output);
        outIm = _mm256_load_ps(output + halfSize);

        leftRe = _mm256_load_ps(left);
        leftIm = _mm256_load_ps(left + halfSize);

        rightRe = _mm256_load_ps(right);
        rightIm = _mm256_load_ps(right + halfSize);

        // Perform the multiply adds
        outRe = _mm256_fmadd_ps(leftRe, rightRe, outRe);   // outRe += leftRe * rightRe
        outRe = _mm256_fnmadd_ps(leftIm, rightIm, outRe);  // outRe -= leftIm * rightIm

        outIm = _mm256_fmadd_ps(leftRe, rightIm, outIm);   // outIm += leftRe * rightIm
        outIm = _mm256_fmadd_ps(leftIm, rightRe, outIm);   // outIm += leftIm * rightRe

        // Write output back to memory. 
        // Reinterpret cast the float location to write back to as __m256
        _mm256_store_ps(output, outRe);
        _mm256_store_ps((output + halfSize), outIm);

        // Advance all the float pointers by 8
        left   += 8;
        right  += 8;
        output += 8;
    }

    // Take care of the 0th bin/Nyquist thing from earlier. 
    _output[0] = tempFloatA;
    _output[halfSize] = tempFloatB;
}

inline void complexMultiplication(float* __restrict output,
                                  const float* __restrict left,
                                  const float* __restrict right,
                                  int numElements,
                                  bool useOptimizedCode,
                                  float& tempA, float& tempB)
{
    if (useOptimizedCode)
        complexMultiplicationSIMD(output, left, right, numElements, tempA, tempB);
    else
    {
        juceVectorComplexMult(output, left, right, numElements);
    }
}