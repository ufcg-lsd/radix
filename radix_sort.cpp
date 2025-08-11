#include <bits/stdc++.h>
using namespace std;
using namespace std::chrono;

const int NUM_PASSES = 8;
const int MAXN = 256;
const int BYTE_MASK = 0xFF;
const uint64_t MSB_MASK = 1ULL << 63;

uint64_t double_to_radix(double d) {
    uint64_t bits;
    memcpy(&bits, &d, sizeof(double));
    if (bits & MSB_MASK)
        bits = ~bits;
    else
        bits ^= MSB_MASK;
    return bits;
}

inline int extract_byte(uint64_t val, int pass) {
    return (val >> (pass << 3)) & BYTE_MASK;
}

void byte_sort(vector<double> &arr, int pass, vector<int>& count, vector<double>& result, vector<uint64_t>& bits) {
    int n = arr.size();

    for (int i = 0; i < n; ++i)
        bits[i] = double_to_radix(arr[i]);

    fill(count.begin(), count.end(), 0);

    for (int i = 0; i < n; ++i) {
        int curr_byte = extract_byte(bits[i], pass);
        ++count[curr_byte];
    }

    for (int i = 1; i < MAXN; ++i)
        count[i] += count[i - 1];

    for (int i = n - 1; i >= 0; --i) {
        int curr_byte = extract_byte(bits[i], pass);
        int idx = --count[curr_byte];
        result[idx] = arr[i];
    }

    swap(arr, result); 
}

void radix_sort(vector<double>& arr) {
    int n = arr.size();
    if (n == 0)
        return;

    vector<int> count(MAXN, 0);
    vector<double> result(n);
    vector<uint64_t> bits(n);

    for (int pass = 0; pass < NUM_PASSES; ++pass)
        byte_sort(arr, pass, count, result, bits);
}

int main() {
    vector<int> sizes = {1, 5, 10, 1000, 1000000, 5000000, 10000000};

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> dis(-1e9, 1e9);

    for (int n : sizes) {
        vector<double> data(n);
        for (int i = 0; i < n; ++i) {
            data[i] = dis(gen);
        }

        cout << "10 primeiros numeros do dataset: ";
        for (int i = 0; i < 10; i++)
            cout << data[i] << " ";
        cout << '\n';;


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

        for (int i = 0; i < 20; i++) 
            cout << copy1[i] << " ";
        cout << endl;

        bool correct = (copy1 == copy2);

        cout << "N = " << n << "\n";
        cout << "Radix Sort: " << radix_time << " s\n";
        cout << "std::sort:  " << stdsort_time << " s\n";
        cout << "Ordenação correta? " << (correct ? "OK" : "NOPE") << "\n\n";
    }

    return 0;
}
