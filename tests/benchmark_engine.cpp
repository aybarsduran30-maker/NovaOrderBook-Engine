#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <algorithm>
#include "OrderBook.hpp"

int main() {
    constexpr size_t WARMUP_ROUNDS = 10000;
    constexpr size_t BENCHMARK_ROUNDS = 100000;

    OrderBook book;

    for (size_t i = 0; i < WARMUP_ROUNDS; ++i) {
        book.addOrder(i, Side::BUY, 10000, 10);
    }

    std::vector<long long> latencies;
    latencies.reserve(BENCHMARK_ROUNDS);

    for (size_t i = 0; i < BENCHMARK_ROUNDS; ++i) {
        uint64_t order_id = WARMUP_ROUNDS + i;
        uint32_t price = 10000 + (i % 50);
        uint32_t qty = 5;

        auto start = std::chrono::high_resolution_clock::now();
        book.addOrder(order_id, Side::BUY, price, qty);
        auto end = std::chrono::high_resolution_clock::now();

        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        latencies.push_back(ns);
    }

    std::sort(latencies.begin(), latencies.end());

    auto p50 = latencies[BENCHMARK_ROUNDS * 50 / 100];
    auto p90 = latencies[BENCHMARK_ROUNDS * 90 / 100];
    auto p99 = latencies[BENCHMARK_ROUNDS * 99 / 100];
    auto p99_9 = latencies[BENCHMARK_ROUNDS * 999 / 1000];

    std::cout << "========================================\n";
    std::cout << " Pure C++ Engine Benchmark              \n";
    std::cout << "========================================\n";
    std::cout << "Samples : " << BENCHMARK_ROUNDS << "\n";
    std::cout << "p50     : " << p50 << " ns\n";
    std::cout << "p90     : " << p90 << " ns\n";
    std::cout << "p99     : " << p99 << " ns\n";
    std::cout << "p99.9   : " << p99_9 << " ns\n";
    std::cout << "========================================\n";

    return 0;
}