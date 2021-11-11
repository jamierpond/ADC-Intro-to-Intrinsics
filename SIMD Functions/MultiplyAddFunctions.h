/*
  ==============================================================================

    MultiplyAddFunctions.h
    Created: 11 Nov 2021 4:41:08pm
    Author:  jamie

  ==============================================================================
*/

#pragma once
#include "SIMDHelpers.h"

inline void multiplyAdd(float* output, const float* left, const float* right, int numElements)
{
    for (int i = 0; i < numElements; i++)
    {
        *output = *left * *right;
        output++;
        left++;
        right++;
    }
}

inline void multiplyAddSIMD(float* __restrict output, const float* __restrict left, const float* __restrict right, int numElements)
{
    int oddNumElementsToDo = numElements % 8;
    if (oddNumElementsToDo != 0)
    {
        multiplyAdd(output, left, right, oddNumElementsToDo);
        output += oddNumElementsToDo;
        left += oddNumElementsToDo;
        right += oddNumElementsToDo;
        numElements -= oddNumElementsToDo;
    }

    auto* outputSIMD = getSIMDPointer(output);
    auto* leftSIMD = getConstSIMDPointer(left);
    auto* rightSIMD = getConstSIMDPointer(right);

    for (int i = 0; i < numElements; i += 8)
    {
        *outputSIMD = _mm256_mul_ps(*leftSIMD, *rightSIMD);
        outputSIMD++;
        leftSIMD++;
        rightSIMD++;
    }
}



