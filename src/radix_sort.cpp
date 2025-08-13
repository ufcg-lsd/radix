#include "radix_sort.h"
#include <vector>
#include <algorithm>
#include <cstdint>
#include <cstring> 

using namespace std;

const int NUM_PASSES = 8;
const int MAXN = 256;
const int BYTE_MASK = 0xFF;
const uint64_t MSB_MASK = 1ULL << 63;

struct RadixBuffers {

    vector<uint64_t> bytes, result_bytes;
    vector<double> result;
    vector<int> count;

    RadixBuffers(int n) {
        bytes.resize(n);
        result_bytes.resize(n);
        result.resize(n);
        count.resize(MAXN);
    }

};

uint64_t double_to_byte(double num) {

    uint64_t byt;
    memcpy(&byt, &num, sizeof(double));

    return (byt & MSB_MASK) ? ~byt : byt ^ MSB_MASK;
}

inline int extract_byte(uint64_t val, int pass) {
    return (val >> (pass << 3)) & BYTE_MASK;
}

void byte_sort(vector<double> &vec, RadixBuffers &buf, int pass) {

    int n = vec.size();
    
    // Reset counter buffer
    fill(buf.count.begin(), buf.count.end(), 0);

    for (int i = 0; i < n; ++i) {
        int curr_byte = extract_byte(buf.bytes[i], pass);
        ++buf.count[curr_byte];
    }

    // Compute the offset of the i-th byte
    for (int i = 1; i < MAXN; ++i) {
        buf.count[i] += buf.count[i - 1];
    }
    
    for (int i = n - 1; i >= 0; --i) {
        int curr_byte = extract_byte(buf.bytes[i], pass);
        int idx = --buf.count[curr_byte];
        buf.result[idx] = vec[i];
        buf.result_bytes[idx] = buf.bytes[i];
    }

    swap(vec, buf.result);
    swap(buf.bytes, buf.result_bytes);
}

void radix_sort(vector<double> &vec) {

    int n = vec.size();
    if (n == 0) {
        return;
    }

    RadixBuffers buf(n);

    // Converts each double value in vec to its byte representation
    // and stores the result in the bytes vector
    transform(vec.begin(), vec.end(), buf.bytes.begin(), double_to_byte);
    
    for (int pass = 0; pass < NUM_PASSES; ++pass) {
        byte_sort(vec, buf, pass);
    }

}
