#include <algorithm>
#include <gtest/gtest.h>
#include <random>
#include <vector>
#include "../src/radix_sort.h" 

using namespace std;
using radix_sort::RadixSort;

TEST(RadixSortTest, SortWhenOnlyPositiveNumbers) {
    vector<double> data = {100.5, 2.1, 55.0, 3.14, 0.0};
    vector<double> expected = {0.0, 2.1, 3.14, 55.0, 100.5};

    RadixSort(data); 

    ASSERT_EQ(data.size(), expected.size());
    for (size_t i = 0; i < data.size(); ++i) {
        EXPECT_DOUBLE_EQ(data[i], expected[i]);
    }
}

TEST(RadixSortTest, SortWhenOnlyNegativeNumbers) {
    vector<double> data = {-5.0, 10.0, -1.5, -100.0, 0.0};
    vector<double> expected = {-100.0, -5.0, -1.5, 0.0, 10.0};

    RadixSort(data);

    for (size_t i = 0; i < data.size(); ++i) {
        EXPECT_DOUBLE_EQ(data[i], expected[i]);
    }
}

TEST(RadixSortTest, SortWhenEmptyVector) {
    vector<double> data = {};

    RadixSort(data);

    EXPECT_TRUE(data.empty());
}

TEST(RadixSortTest, SortWhenSingleElement) {
    vector<double> data = {42.42};

    RadixSort(data);

    EXPECT_EQ(data.size(), 1);
    EXPECT_DOUBLE_EQ(data[0], 42.42);
}

TEST(RadixSortTest, SortWhenLargeRandomDataset) {
    size_t n = 10000;
    vector<double> data(n);
    
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> dis(-1000.0, 1000.0);

    for(size_t i = 0; i < n; ++i) {
        data[i] = dis(gen);
    }

    vector<double> expected = data;
    sort(expected.begin(), expected.end());

    RadixSort(data);

    for(size_t i = 0; i < n; i++) {
        EXPECT_DOUBLE_EQ(data[i], expected[i]) << "Erro no índice: " << i << "\n";
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}