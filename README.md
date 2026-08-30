 NovaOrderBook

An ultra-low-latency, single-threaded **Limit Order Book (LOB) matching engine** built with modern **C++20**. Engineered to process high-throughput financial market orders with sub-microsecond execution times.



 Overview

Financial exchanges require deterministic and ultra-fast order processing. **NovaOrderBook** implements core exchange-level matching algorithms with a heavy focus on cache locality and zero runtime heap allocation.

 Key Highlights
Deterministic Latency:** Sub-200ns order placement and matching latency.
Zero Runtime Dynamic Allocation:** Uses pre-allocated memory pools to prevent OS heap contention.
Modern C++20 Standard:** Clean, idiomatic modern C++ architecture.
Tested & Benchmarked:** Validated on automated test suites with 1,000,000 randomized market orders.



 Performance Benchmarks

Stress tests executed on **1M randomized buy/sell orders**:

 Metric | Base Implementation (`std::map`) Optimized (Memory Pool + Flat Array) Improvement 
|

Avg Latency**  `510.29 ns`  **`158.40 ns`**  **~3.2x Faster** 
Throughput**  `1.95M ops/sec`  **`6.31M ops/sec`**  **+223%** 
Runtime (1M Orders)** `0.5103 s`  **`0.1584 s`**   **69% time reduced** |
Heap Memory Usage**  Dynamic ($O(\log N)$ tree nodes)  Fixed ($O(1)$ pre-allocated pool)  **Zero runtime allocations** 



#  Technical Architecture & Design Decisions

* **Price Ladder ($O(1)$ Lookup):** Shifted from binary search trees (`std::map`) to a contiguous flat array indexed directly by price ticks.
* **Memory Pool Architecture:** Pre-allocated a contiguous buffer for active orders, eliminating runtime `malloc`/`free` calls during execution.
* **Intrusive Linked List:** FIFO priority order queues are linked directly inside the `Order` struct to maximize CPU L1/L2 cache hits.



# Quick 

# Prerequisites
* `g++` (C++20 compatible) or any modern C++ compiler.

 Build & Run Benchmark
```bash
# 1. Compile the benchmark suite
g++ -std=c++20 -O3 -march=native -Iinclude tests/benchmark.cpp src/OrderBook.cpp -o nova_benchmark.exe

# 2. Run the engine
./nova_benchmark.exe

NovaOrderBook-Engine/
├── include/
│   ├── Order.hpp         # Order definitions and memory structures
│   └── OrderBook.hpp     # Engine declarations and memory pool
├── src/
│   ├── main.cpp          # Demo execution pipeline
│   └── OrderBook.cpp     # Matching logic & array ladder engine
├── tests/
│   └── benchmark.cpp     # High-throughput benchmark suite
└── README.md


