#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include<vector>

#include "external_sort.h"
#include "radix_sort.h"

using namespace std;
using external_sort::SortStrategy;
using radix_sort::RadixSort;

namespace external_sort {
    // Defines a safety margin of the maximum memory to be used to store
    // the files generated during the run. It is 90% of the memory requested
    // by the user.
    const double SAFETY_MARGIN = 0.9;

    namespace {
        // Structure for the priority queue
        struct MinHeapNode {
            double value;
            int file_index; // to tell which file it came from

            bool operator>(const MinHeapNode& other) const {
                return value > other.value;
            }
        };
    }

    ExternalSort::ExternalSort(string input_file, string output_file,
                               size_t memory_limit_mb, SortStrategy sort_strategy) {
        this->input_file = input_file;
        this->output_file = output_file;
        this->sort_strategy = sort_strategy;
        // Converts MB to Bytes and set up the memory limit
        this->memory_limit_bytes = memory_limit_mb * 1024 * 1024;
    }

    void ExternalSort::SortInMemory(vector<double>& buffer) {
        if (this->sort_strategy == RADIX_SORT) {
            RadixSort(buffer); 
            return;
        } 

        std::sort(buffer.begin(), buffer.end());
    }

    vector<string> ExternalSort::GenerateRuns() {
        ifstream in(input_file, ios::binary);

        if (!in) {
            cerr << "Couldn't open the input file: " << input_file << '\n';
            return {};
        }

        vector<string> run_files;

        // Compute buffer size
        size_t safe_payload = (size_t) (this->memory_limit_bytes * SAFETY_MARGIN);
        // limit Radix usage to make the comparison between the algorithms fair
        size_t memory_usage_factor = (this->sort_strategy == RADIX_SORT) ? 3 : 1;
        size_t max_elements = safe_payload / (memory_usage_factor *sizeof(double));

        vector<double> buffer;
        buffer.reserve(max_elements);

        double val;
        int runCount = 0;
        while (in.read(reinterpret_cast<char*>(&val), sizeof(double))) {
            buffer.push_back(val);
            if (buffer.size() < max_elements) {
                continue;
            }

            SortInMemory(buffer); 

            string temp_name = "temp_run_" + to_string(runCount++) + ".bin";
            ofstream out(temp_name, ios::binary);
            out.write(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(double));
            
            run_files.push_back(temp_name);
            buffer.clear();
        }

        if (!buffer.empty()) {
            SortInMemory(buffer);
            string temp_name = "temp_run_" + to_string(runCount++) + ".bin";
            ofstream out(temp_name, ios::binary);
            out.write(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(double));
            run_files.push_back(temp_name);
        }

        return run_files;
    }

    void ExternalSort::MergeRuns(const vector<std::string> &run_files) {
        priority_queue<MinHeapNode, vector<MinHeapNode>, greater<MinHeapNode>> pq;
        vector<ifstream*> inputs;

        for (int i = 0; i < (int) run_files.size(); i++) {
            ifstream* file = new ifstream(run_files[i], ios::binary);

            double val;
            if (file->read(reinterpret_cast<char*>(&val), sizeof(double))) {
                pq.push({val, (int)i});
                inputs.push_back(file);
            } else {
                delete file;
                inputs.push_back(nullptr);
            }
        }

        ofstream finalOut(output_file, ios::binary);

        while (!pq.empty()) {
            MinHeapNode node = pq.top();
            pq.pop();

            finalOut.write(reinterpret_cast<char*>(&node.value), sizeof(double));

            if (inputs[node.file_index] 
                && inputs[node.file_index]->read(reinterpret_cast<char*>(&node.value), sizeof(double))) {
                pq.push({node.value, node.file_index});
            }
        }

        for (int i = 0; i < (int) inputs.size(); i++) {
            if (inputs[i]) {
                inputs[i]->close();
                delete inputs[i];
                remove(run_files[i].c_str());
            }
        }
    }

    void ExternalSort::Run() {
        MergeRuns(GenerateRuns());
    }

}   // external_sort
