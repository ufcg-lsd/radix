#include <algorithm>
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <random>
#include <vector>

#include "external_sort.h"
#include "radix_sort.h" 

using namespace std;
using namespace external_sort;

namespace {

    class ExternalSortTest : public ::testing::Test {
        protected:
            const string kInputFile = "test_input.bin";
            const string kOutputFile = "test_output.bin";

            void TearDown() override {
                remove(kInputFile.c_str());
                remove(kOutputFile.c_str());
                for(int i=0; i<100; i++) {
                    string temp = "temp_run_" + to_string(i) + ".bin";
                    remove(temp.c_str());
                }
            }

            void GenerateBinaryFile(size_t num_elements) {
                ofstream out(kInputFile, ios::binary);
            
                random_device rd;
                mt19937 gen(rd());
                uniform_real_distribution<> dis(-1000.0, 1000.0);

                for (size_t i = 0; i < num_elements; i++) {
                    double val = dis(gen);
                    out.write(reinterpret_cast<const char*>(&val), sizeof(double));
                }
            }

            vector<double> ReadOutput() {
                ifstream in(kOutputFile, ios::binary);
                vector<double> res;
                double val;
                while (in.read(reinterpret_cast<char*>(&val), sizeof(double))) {
                    res.push_back(val);
                }

                return res;
            }

            bool IsSorted(const vector<double>& data) {
                return is_sorted(data.begin(), data.end());
            }
    };

    TEST_F(ExternalSortTest, SortsSmallDatasetInMemory) {
        GenerateBinaryFile(100); 

        ExternalSort sorter(kInputFile, kOutputFile, 100, SortStrategy::STD_SORT);
        sorter.Run();

        vector<double> result = ReadOutput();
    
        ASSERT_EQ(result.size(), 100);
        EXPECT_TRUE(IsSorted(result));
    }

    TEST_F(ExternalSortTest, SortsLargeDatasetWithLowMemory) {
        size_t num_elements = 5000;
        GenerateBinaryFile(num_elements);

        ExternalSort sorter(kInputFile, kOutputFile, 0.005, SortStrategy::STD_SORT);
        sorter.Run();

        vector<double> result = ReadOutput();

        ASSERT_EQ(result.size(), num_elements);
        EXPECT_TRUE(IsSorted(result));
    }

    TEST_F(ExternalSortTest, UsesRadixSortStrategyCorrectly) {
        GenerateBinaryFile(500);

        ExternalSort sorter(kInputFile, kOutputFile, 10, SortStrategy::RADIX_SORT);
        sorter.Run();

        vector<double> result = ReadOutput();

        ASSERT_EQ(result.size(), 500);
        EXPECT_TRUE(IsSorted(result));
    }

    TEST_F(ExternalSortTest, UsesRadixSortWhenLargeDatasetWithLowMemory) {
        size_t num_elements = 5000;
        GenerateBinaryFile(num_elements);

        ExternalSort sorter(kInputFile, kOutputFile, 0.005, SortStrategy::RADIX_SORT);
        sorter.Run();

        vector<double> result = ReadOutput();

        ASSERT_EQ(result.size(), num_elements);
        EXPECT_TRUE(IsSorted(result));
    }

} // namespace