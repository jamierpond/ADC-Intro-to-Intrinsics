/*
  ==============================================================================

    MultiplyAddFunctions.h
    Created: 11 Nov 2021 4:41:08pm
    Author:  jamie

  ==============================================================================
*/

#pragma once
#include "SIMDHelpers.h"

inline void multiply(float* output, const float* left, const float* right, int numElements)
{
    for (int i = 0; i < numElements; i++)
    {
        output[i] = left[i] * right[i];
    }
}

inline void multiplySIMD(float* __restrict output, const float* __restrict left, const float* __restrict right, int numElements)
{
    int oddNumElementsToDo = numElements % 8;
    if (oddNumElementsToDo != 0)
    {
        multiply(output, left, right, oddNumElementsToDo);
        output += oddNumElementsToDo;
        left   += oddNumElementsToDo;
        right  += oddNumElementsToDo;
        numElements -= oddNumElementsToDo;
    }

    __m256 _out, _left, _right;

    const int numSIMD = numElements / 8;

    for (int i = 0; i < numSIMD; i++)
    {
        _out   = _mm256_load_ps(output);
        _left  = _mm256_load_ps(left);
        _right = _mm256_load_ps(right);
        _out   = _mm256_mul_ps(_left, _right);
        _mm256_store_ps(output, _out);

        output += 8;
        left   += 8;
        right  += 8;
    }
}



