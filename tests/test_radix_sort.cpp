#include "../src/radix_sort.h"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <cmath>

using namespace std;
using namespace std::chrono;

const double EPS = 1e-9;        // a very small number

bool approx_equal(double x, double y) {
    return fabs(x - y) < EPS;
}

bool is_sorted(vector<double> vec, vector<double> sorted) {
    if (vec.size() != sorted.size()) {
        return false;
    }

    for (int i = 0; i < vec.size(); ++i) {
        if (!approx_equal(vec[i], sorted[i])) {
            return false;
        }
    }

    return true;
}

int main() {
    vector<int> sizes = {10, 1000, 1000000, 5000000, 10000000};

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> dis(-1e9, 1e9);

    for (int n : sizes) {
        vector<double> data(n);
        for (int i = 0; i < n; ++i) {
            data[i] = dis(gen);
        }

        cout << "First 10 numbers of the dataset: ";
        for (int i = 0; i < min(10, n); i++)
            cout << data[i] << " ";
        cout << '\n';


        auto copy1 = data;
        auto copy2 = data;
        
        auto start = high_resolution_clock::now();
        radix_sort(copy1);
        auto end = high_resolution_clock::now();
        double radix_time = duration_cast<duration<double>>(end - start).count();

        start = high_resolution_clock::now();
        sort(copy2.begin(), copy2.end());
        end = high_resolution_clock::now();
        double stdsort_time = duration_cast<duration<double>>(end - start).count();

        bool ok = is_sorted(copy1, copy2);
        string winner = (radix_time < stdsort_time) ? "Radix Sort" : "std::sort";

        cout << "N = " << n << "\n";
        cout << "Radix Sort: " << radix_time << " s\n";
        cout << "std::sort:  " << stdsort_time << " s\n";
        cout << "Is sorted? " << (ok ? "Yes" : "No") << "\n";
        cout << "Winner: " << winner << "\n\n";
    }

    return 0;
}
