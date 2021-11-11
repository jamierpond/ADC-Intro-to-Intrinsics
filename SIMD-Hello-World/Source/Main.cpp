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
