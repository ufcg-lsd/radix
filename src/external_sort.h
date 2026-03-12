#pragma once
#include <string>
#include <vector>

namespace external_sort {
    // Represents the sort strategy that is used to sort an segment
    // during the extenal sort routine.
    enum SortStrategy {
        RADIX_SORT,
        STD_SORT,
    };

    class ExternalSort {
        public:
            // Given an input file, this constructor sets up the external sort
            // procedure. The memory_limit param is used to set the size of each
            // segment in RAM, and the sort_strategy is the sort strategy that is
            // used to sort each segment.
            ExternalSort(std::string input_file, std::string output_file, 
                        size_t memory_limit_mb, SortStrategy sort_strategy);

            // Run the external sort routine.
            void Run();

        private:
            std::string input_file;
            std::string output_file;
            size_t memory_limit_bytes;
            SortStrategy sort_strategy;

            // Split the input_file into sorted files (runs)
            std::vector<std::string> GenerateRuns();
           
            // This function takes the generated runs and merges it by applying the
            // k-way merge sort algorithm
            void MergeRuns(const std::vector<std::string> &runs);

            // Function to select which sort strategy is used to sort in-memory
            void SortInMemory(std::vector<double> &buffer);
    };

}   // external_sort