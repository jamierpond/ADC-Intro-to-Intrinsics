/*
  ==============================================================================

    SIMDBasics.h
    Created: 11 Nov 2021 5:56:02pm
    Author:  Jamie

  ==============================================================================
*/

#pragma once
#include "SIMDHelpers.h"


inline void fillWithZerosBasic(float* __restrict data, int numElements)
{
    for (int i = 0; i < numElements; i++)
    {
        data[i] = 0.0f;
    }
}

inline void fillWithZerosSIMD(float* __restrict data, int numElements)
{
    int oddNumElementsToDo = numElements % 8;
    if (oddNumElementsToDo != 0)
    {
        fillWithZerosBasic(data, oddNumElementsToDo);
        data += oddNumElementsToDo;
        numElements -= oddNumElementsToDo;
    }

    int numSIMD = numElements / 8;

    __m256 value;

    for (int i = 0; i < numSIMD; i++)
    {
        value = _mm256_load_ps(data);
        value = _mm256_setzero_ps();
        _mm256_store_ps(data, value);
        data += 8;
    }
}

inline void fillWithZeros(float* __restrict data, int numElements, bool shouldUseOptimzed)
{
    if (shouldUseOptimzed)
        fillWithZerosSIMD(data, numElements);
    else
        fillWithZerosBasic(data, numElements);
}

inline void multiply(float* data, float multiplier, int numElements)
{
    jassert(numElements % 8 == 0);
    __m256 mult = _mm256_set1_ps(multiplier);

    __m256 value;

    const int numSIMD = numElements / 8;

    for (int i = 0; i < numSIMD; i++)
    {
        value = _mm256_load_ps(data);
        value = _mm256_mul_ps(value, mult);
        _mm256_store_ps(data, value);
        data += 8;
    }
}