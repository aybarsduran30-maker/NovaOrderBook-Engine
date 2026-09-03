# NovaOrderBook

An ultra-low-latency, single-threaded Limit Order Book matching engine built with modern C++20.

## Performance Benchmarks

Stress tests executed on 1,000,000 randomized buy/sell orders:

| Metric | Base Implementation (std::map) | Optimized (Memory Pool + Flat Array) |
| --- | --- | --- |
| Avg Latency | 510.29 ns | 158.40 ns |
| Throughput | 1.95M ops/sec | 6.31M ops/sec |
| Runtime (1M Orders) | 0.5103 s | 0.1584 s |

## Technical Architecture

- Price Ladder (O(1) Lookup): Contiguous flat array indexed directly by price ticks.
- Memory Pool Architecture: Pre-allocated buffer for orders, zero runtime allocations.
- Intrusive Linked List: FIFO priority order queues linked inside Order struct.

## Build and Run

```bash
g++ -std=c++20 -O3 -march=native -Iinclude tests/benchmark.cpp src/OrderBook.cpp -o nova_benchmark.exe
./nova_benchmark.exe
* **Python Bindings & Terminal UI:** Integrated C++20 engine with Python via Pybind11 and implemented a live terminal-based order book depth stream (`simulate_stream.py`) visualizing real-time bid/ask spreads, market depth, and executions.
* **Performance Validation:** Validated real-time event streaming and dynamic cancellations alongside core benchmark results (~9.28M orders/sec throughput, ~107.66 ns latency).
