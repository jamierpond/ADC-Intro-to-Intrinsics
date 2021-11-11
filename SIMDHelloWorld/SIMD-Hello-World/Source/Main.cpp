/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include <immintrin.h>

void fillWithAscendingNumbers(std::vector<float>& v, int startingAt = 1)
{
    for (auto& number : v) { number = (float)startingAt++; }
}

__m256* getSIMDPointer(float* floatPointer)
{
    return reinterpret_cast<__m256*>(floatPointer);
}

void addVectors(float* __restrict output, float* __restrict left, float* __restrict right, int numElements)
{
    for (int i = 0; i < numElements; i++)
    {
        *output++ = *left++ + *right++;
    }
}

void addVectorsSIMD(float* __restrict output, float* __restrict left, float* __restrict right, int numElements)
{
    // This is best practice if you can't make assumptions about how many inputs you're going to have!
    int oddSamplesToDo = numElements % 8;
    if (oddSamplesToDo != 0)
    {
        addVectors(output, left, right, oddSamplesToDo);
        output += oddSamplesToDo;
        left   += oddSamplesToDo;
        right  += oddSamplesToDo;
    }

    auto* outputSIMD = getSIMDPointer(output);
    auto* leftSIMD   = getSIMDPointer(left);
    auto* rightSIMD  = getSIMDPointer(right);

    int numSIMDOperations = numElements / 8;

    for (int i = 0; i < numSIMDOperations; i++)
    {
        *outputSIMD++ = _mm256_add_ps(*leftSIMD++, *rightSIMD++);
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
