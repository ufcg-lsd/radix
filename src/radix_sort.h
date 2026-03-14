#pragma once
#include <vector>

namespace radix_sort {

    // Apply the optimized Radix Sort routine to sort an array
    // of doubles in non-decreasing order. This procedure only works
    // if the double follows the IEE754 pattern
    void RadixSort(std::vector<double> &arr);

}   // radix_sort
