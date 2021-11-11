/*
  ==============================================================================

    SIMDHelpers.h
    Created: 11 Nov 2021 6:49:44pm
    Author:  Jamie

  ==============================================================================
*/

#pragma once
#include <immintrin.h>
#include "JuceHeader.h"

inline __m256* getSIMDPointer(float* floatPointer)
{
    return reinterpret_cast<__m256*>(floatPointer);
}

inline const __m256* getConstSIMDPointer(const float* floatPointer)
{
    return reinterpret_cast<const __m256*>(floatPointer);
}
