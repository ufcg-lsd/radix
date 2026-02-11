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

inline double uint64_to_double(uint64_t num) {
    // Converts uint64_t back to double
    uint64_t byt = (num & MSB_MASK) ? num ^ MSB_MASK : ~num;

    double res;
    memcpy(&res, &byt, sizeof(double));
    return res;
}

void radix_sort::RadixSort(vector<double> &arr) {
    int n = (int) arr.size();

    if (n < 2) {
        return;
    }

    vector<uint64_t> keys(n);
    for (int i = 0; i < n; i++) {
        keys[i] = double_to_uint64(arr[i]);
    }

    vector<uint64_t> temp_buffer(arr.size());
    vector<uint64_t> *src = &keys;
    vector<uint64_t> *dest = &temp_buffer;

    for (int p = 0; p < NUM_PASSES; p++) {
        size_t count[RADIX] = {0};
        
        int shift = p << BITS_PER_BYTE_SHIFT;

        // Prefix sum to compute offset
        for (int i = 0; i < n; i++) {
            int byte_idx = ((*src)[i] >> shift) & BYTE_MASK;
            count[byte_idx]++;
        }
        for (int i = 1; i < RADIX; i++) {
            count[i] += count[i - 1];
        }

        // Place number in the current spot
        for (int i = n - 1; i >= 0; i--) {
            int byte_idx = ((*src)[i] >> shift) & BYTE_MASK;
            (*dest)[--count[byte_idx]] = (*src)[i];
        }

        swap(src, dest);
    }
    
    for (int i = 0; i < n; i++) {
        arr[i] = uint64_to_double((*src)[i]);
    }
}
