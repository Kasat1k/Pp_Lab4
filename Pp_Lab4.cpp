#include <iostream>
#include <random>
#include <thread>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <execution>
#include <chrono>
#include <vector>
#include <functional>

using namespace std;
void fillVecRandomNum(std::vector<int>& vec, int minValue, int maxValue) {
    random_device rd;  
    mt19937 mt(rd()); 
    uniform_int_distribution<int> dist(minValue, maxValue); 

    for (int& num : vec) {
        num = dist(mt); 
    }
}

void output(const string& policyName, double time) {
    cout << policyName << ": " << time << " ms\n";
}

template<typename Func>
auto timeMeasure(const std::string& description, Func func) {
    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    auto result = func();
    auto end = high_resolution_clock::now();
    duration<double, std::milli> timeMeasure = end - start;
    output(description, timeMeasure.count());
    return std::make_pair(result, timeMeasure.count());
}

int parallelMin(const vector<int>& data, int k) {
    vector<thread> threads;
    vector<int> results(k, numeric_limits<int>::max());
    int workSize = data.size() / k + (data.size() % k != 0);
    mutex results_mutex;

    auto th = [&data, &results, &results_mutex](int index, int start, int end) {
        auto local_min = *min_element(data.begin() + start, data.begin() + end);
        lock_guard<mutex> lock(results_mutex);
        results[index] = local_min;
    };

    for (int i = 0; i < k; ++i) {
        int start = i * workSize;
        int reuslt_final = min(start + workSize, static_cast<int>(data.size()));
        threads.emplace_back(th, i, start, reuslt_final);
    }

    for (auto& t : threads) {
        t.join();
    }

    return *min_element(results.begin(), results.end());
}

int findMin(const vector<int>& data, string policy) {
    auto result = timeMeasure(policy, [&]() {
        if (policy == "Without policy") {
            return *min_element(data.begin(), data.end());
        }
        if (policy == "Parallel") {
            return *min_element(execution::par, data.begin(), data.end());
        }
        if (policy == "UnSequential") {
            return *min_element(execution::unseq, data.begin(), data.end());
        }
        if (policy == "Parallel_unseq") {
            return *min_element(execution::par_unseq, data.begin(), data.end());
        }
        });
    return result.first;
}

int main() {

    vector<int> data(1000000);
    fillVecRandomNum(data, 1, 2000000);
    vector<int> dataCopy(1000000);
    dataCopy = data;
    string policy;
    policy = "Without policy";
    findMin(data, policy);
    data = dataCopy;
    policy = "Parallel";
    findMin(data, policy);
    data = dataCopy;
    policy = "Parallel_unseq";
    findMin(data, policy);
    data = dataCopy;
    policy = "UnSequential";
    findMin(data, policy);
    data = dataCopy;
   
    cout << "Custom parallel algorithm results:\n";
    cout << "\t\tK    Time \n";

    int bestK = 0;
    double bestTime = numeric_limits<double>::max();

    for (int k = 1; k <= thread::hardware_concurrency(); ++k) {
        auto result = timeMeasure("Custom function K=" + to_string(k), [&]() {
            return parallelMin(data, k);
            });

        if (result.second < bestTime) {
            bestTime = result.second;
            bestK = k;
        }
    }

    cout << "Best K: " << bestK << " with Time (ms): " << bestTime << endl;

    return 0;

}
