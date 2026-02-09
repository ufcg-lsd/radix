#include <algorithm>
#include <cstdint>
#include <cstring> 
#include <vector>
#include "radix_sort.h"

using namespace std;

// Required number of passes to go through each byte of the type double (64 bits = 8 bytes).
const int NUM_PASSES = sizeof(double);

// The radix (base): 256 because we process 1 byte at a time.
const int RADIX = 256;

// Mask to extract the byte we want to analyze (or sort)
const int BYTE_MASK = 0xFF;

// Mask to extract the MSB and to check if the number is positive or negative
const uint64_t MSB_MASK = 1ULL << 63;

// Number of bits we need to shift to go to the next byte (1 << BITS_PER_BYTE_SHIFT = 8)
const int BITS_PER_BYTE_SHIFT = 3;

// Converts a double to a byte (uint64)
inline uint64_t double_to_uint64(double num) {
    uint64_t byt;
    memcpy(&byt, &num, sizeof(double));

    return (byt & MSB_MASK) ? ~byt : byt ^ MSB_MASK;
}

void radix_sort::RadixSort(vector<double> &arr) {
    int n = (int) arr.size();

    if (n < 2) {
        return;
    }

    // Temporary buffer to store sorted bytes
    vector<double> temp_buffer(arr.size());
        
    vector<double> *src = &arr;
    vector<double> *dest = &temp_buffer;

    for (int p = 0; p < NUM_PASSES; p++) {
        // Compute frequency of each element
        size_t count[RADIX];
        memset(count, 0, sizeof(count));
        for (int i = 0; i < n; i++) {
            uint64_t byte_val = double_to_uint64((*src)[i]);
            int byte_idx = (byte_val >> (p << BITS_PER_BYTE_SHIFT)) & BYTE_MASK;
            count[byte_idx]++;
        }

        // Compute offset
        for (int i = 1; i < RADIX; i++) {
            count[i] += count[i - 1];
        }

        for (int i = n - 1; i >= 0; i--) {
            uint64_t byte_val = double_to_uint64((*src)[i]);
                
            int byte_idx = (byte_val >> (p << BITS_PER_BYTE_SHIFT)) & BYTE_MASK;
            int pos = --count[byte_idx];
            (*dest)[pos] = (*src)[i];
        }

        swap(src, dest);
    }

    if (src != &arr) {
        arr = move(*src);
    }
}
