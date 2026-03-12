/*
    Runs the external sort routine. Allows the user to configure
    which sort routine to use (std::sort or optimized radix sort).

    Arguments (Positional - Order Matters):
        1. input_file:    Path to the binary file to be sorted.
        2. output_file:   Path where the sorted result will be stored.
        3. memory_mb:     Memory limit in MB for the internal buffer.
        4. sort_strategy: Algorithm to use ("std" or "radix").

    Usage:
        ./main <input_file> <output_file> <memory_mb> <sort_strategy>

    Examples:
        # Use defaults (input.bin, output.bin, 100MB, std):
        ./main

        # Custom files, default memory/strategy:
        ./main my_data.bin my_result.bin

        # Full configuration (512MB RAM using Radix Sort):
        ./main large_data.bin sorted.bin 512 radix
*/

#include <chrono>
#include <iostream>
#include <string>

#include "external_sort.h"

using namespace std;
using namespace external_sort;

int main(int argc, char* argv[]) {
    string input_file = "input.bin";
    string output_file = "output.bin";
    double memory_MB = 100.0; 
    string sort_strategy = "std";

    if (argc > 1) {
        input_file = argv[1];
    }

    if (argc > 2) {
        output_file = argv[2];
    }

    if (argc > 3) {
        try {
            memory_MB = stod(argv[3]);
        } catch (...) {
            return 1;
        }
    }

    if (argc > 4) {
        sort_strategy = argv[4];
    }

    SortStrategy strategy = (sort_strategy == "radix") 
        ? SortStrategy::RADIX_SORT : SortStrategy::STD_SORT;

    auto start = chrono::steady_clock::now();

    ExternalSort sorter(input_file, output_file,
            memory_MB, strategy);

    sorter.Run();

    auto end = chrono::steady_clock::now();
    chrono::duration<double> diff = end - start;

    // I dont like endls ;)
    cout << "Execution time: " << diff.count() << " s" << '\n';
    return 0;
}