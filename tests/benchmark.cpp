#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>
#include "OrderBook.hpp"

struct RawOrder {
    uint64_t id;
    Side side;
    uint32_t price;
    uint32_t count;
};

int main() {
    const size_t NUM_ORDERS = 1000000;
    OrderBook book;

    std::vector<RawOrder> testOrders;
    testOrders.reserve(NUM_ORDERS);

    std::mt19937_64 rng(1337);
    std::uniform_int_distribution<uint32_t> priceDist(OrderBook::MIN_PRICE, OrderBook::MAX_PRICE);
    std::uniform_int_distribution<uint32_t> qtyDist(1, 100);
    std::uniform_int_distribution<int> sideDist(0, 1);

    for (size_t i = 1; i <= NUM_ORDERS; ++i) {
        Side side = (sideDist(rng) == 0) ? Side::BUY : Side::SELL;
        uint32_t price = priceDist(rng);
        uint32_t count = qtyDist(rng);
        testOrders.push_back({i, side, price, count});
    }

    std::cout << "Starting Ultra-Low-Latency Benchmark with " << NUM_ORDERS << " orders..." << std::endl;

    auto startTime = std::chrono::high_resolution_clock::now();

    for (const auto& o : testOrders) {
        book.addOrder(o.id, o.side, o.price, o.count);
    }

    auto endTime = std::chrono::high_resolution_clock::now();

    auto totalDurationNs = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();
    double totalDurationSec = totalDurationNs / 1e9;
    double opsPerSec = static_cast<double>(NUM_ORDERS) / totalDurationSec;
    double avgLatencyNs = static_cast<double>(totalDurationNs) / NUM_ORDERS;

    std::cout << "\n================ OPTIMIZED BENCHMARK RESULTS ================" << std::endl;
    std::cout << "Total Orders Processed : " << NUM_ORDERS << std::endl;
    std::cout << "Total Trades Executed  : " << book.getTradeCount() << std::endl;
    std::cout << "Total Execution Time   : " << std::fixed << std::setprecision(4) << totalDurationSec << " seconds" << std::endl;
    std::cout << "Throughput             : " << std::fixed << std::setprecision(2) << opsPerSec << " orders/sec" << std::endl;
    std::cout << "Average Latency        : " << std::fixed << std::setprecision(2) << avgLatencyNs << " ns/order" << std::endl;
    std::cout << "=============================================================" << std::endl;

    return 0;
}