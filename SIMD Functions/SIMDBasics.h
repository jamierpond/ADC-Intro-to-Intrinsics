/*
  ==============================================================================

    SIMDBasics.h
    Created: 11 Nov 2021 5:56:02pm
    Author:  Jamie

  ==============================================================================
*/

#pragma once
#include "SIMDHelpers.h"
//
//// This is a 'supporting function' to addVectorsSIMD. 
//void addVectors(float* __restrict output, float* __restrict left, float* __restrict right, int numElements)
//{
//    for (int i = 0; i < numElements; i++)
//    {
//        output[i] = left[i] + right[i];
//    }
//}
//
//// Adds two vectors to an output, using AVX optimizations. 
//void addVectorsSIMD(float* __restrict output, float* __restrict left, float* __restrict right, int numElements)
//{
//    int oddSamplesToDo = numElements % 8;
//    if (oddSamplesToDo != 0)
//    {
//        addVectors(output, left, right, oddSamplesToDo);
//        output += oddSamplesToDo;
//        left   += oddSamplesToDo;
//        right  += oddSamplesToDo;
//    }
//
//    auto* outputSIMD = getSIMDPointer(output);
//    auto* leftSIMD = getConstSIMDPointer(left);
//    auto* rightSIMD = getConstSIMDPointer(right);
//
//    for (int i = 0; i < numElements; i += 8)
//    {
//        *outputSIMD = _mm256_add_ps(*leftSIMD, *rightSIMD);
//        outputSIMD++;
//        leftSIMD++;
//        rightSIMD++;
//    }
//}

inline void fillWithZeros(float* __restrict data, int numElements)
{
    for (int i = 0; i < numElements; i++)
    {
        *data = 0.0f;
        data++;
    }
}

inline void fillWithZerosSIMD(float* __restrict data, int numElements)
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
        *dataSIMD = _mm256_setzero_ps();
        dataSIMD++;
    }
}

