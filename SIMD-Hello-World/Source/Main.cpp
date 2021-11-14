/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include <immintrin.h>
#include "../../SIMD Functions/PondSIMDFunctions.h"

void fillWithAscendingNumbers(std::vector<float>& v, int startingAt = 1)
{
    for (auto& number : v) { number = (float)startingAt++; }
}

// This is a 'supporting function' to addVectorsSIMD. 
void addVectors(float* __restrict output, float* __restrict left, float* __restrict right, int numElements)
{
    for (int i = 0; i < numElements; i++)
    {
        output[i] = left[i] + right[i];
    }
}

// Adds two vectors to an output, using AVX optimizations. 
void addVectorsSIMD(float* __restrict output, float* __restrict left, float* __restrict right, int numElements)
{
    register __m256 _out, _left, _right;

    for (int i = 0; i < numElements-8; i += 8)
    {
        _out   = _mm256_load_ps(&output[i]);
        _left  = _mm256_load_ps(&left[i]);
        _right = _mm256_load_ps(&right[i]);
        _out = _mm256_add_ps(_left, _right);
        _mm256_store_ps(&output[i], _out);
    }
}

//==============================================================================
int main (int argc, char* argv[])
{
    int numElements = 16;
    std::vector<float> left(numElements);
    std::vector<float> right(numElements);
    std::vector<float> output(numElements);

    fillWithAscendingNumbers(left);
    fillWithAscendingNumbers(right);

    addVectorsSIMD(output.data(), left.data(), right.data(), numElements);

    for (int i = 0; i < numElements; i++)
    {
        std::cout << output[i] << std::endl;
    }

    return 0;
}

//
//// Adds two vectors to an output, using AVX optimizations. 
//void addVectorsSIMD(float* __restrict output, float* __restrict left, float* __restrict right, int numElements)
//{
//    int oddSamplesToDo = numElements % 8;
//    if (oddSamplesToDo != 0)
//    {
//        addVectors(output, left, right, oddSamplesToDo);
//        output += oddSamplesToDo;
//        left += oddSamplesToDo;
//        right += oddSamplesToDo;
//        numElements -= oddSamplesToDo;
//    }
//
//    auto* outputSIMD = getSIMDPointer(output);
//    auto* leftSIMD = getConstSIMDPointer(left);
//    auto* rightSIMD = getConstSIMDPointer(right);
//
//    int numSIMD = numElements / 8;
//
//    for (int i = 0; i < numSIMD; i++)
//    {
//        outputSIMD[i] = _mm256_add_ps(leftSIMD[i], rightSIMD[i]);
//    }
//}