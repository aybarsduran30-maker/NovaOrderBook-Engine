# NovaOrderBook

An ultra-low-latency, single-threaded Limit Order Book (LOB) matching engine built with modern C++20 and optimized for sub-microsecond high-frequency trading (HFT) environments.

##  Performance Benchmarks

Stress tests executed on 1,000,000 randomized buy/sell/cancel orders (Hardware: Native x86_64, GCC -O3):

| Metric | Base Implementation (`std::map`) | Optimized (Memory Pool + Flat Array) | Python API (`Pybind11` Stream) |
| --- | --- | --- | --- |
| **Avg Latency** | 510.29 ns | **158.40 ns** | **107.66 ns** (Event Loop) |
| **Throughput** | 1.95M ops/sec | **6.31M ops/sec** | **9.28M ops/sec** (Max burst) |
| **Runtime (1M)** | 0.5103 s | **0.1584 s** | — |

---

##  Technical Architecture

* **Price Ladder ($O(1)$ Lookup):** Contiguous flat array indexed directly by price ticks, completely bypassing tree traversal overhead.
* **Memory Pool Architecture:** Custom pre-allocated arena buffer for `Order` nodes, enforcing **zero runtime dynamic allocations (`malloc`/`new`)** on the critical matching path.
* **Intrusive Doubly-Linked List:** FIFO priority order queues linked directly inside the `Order` struct to eliminate pointer chasing and cache misses.
* **Python Bindings & Terminal UI:** Native C++20 engine exposed to Python via `Pybind11`. Includes a real-time terminal dashboard (`simulate_stream.py`) visualizing bid/ask spreads, market depth, and execution flows.

---

##  Build and Run

### C++ Benchmark
```bash
g++ -std=c++20 -O3 -march=native -Iinclude tests/benchmark.cpp src/OrderBook.cpp -o nova_benchmark.exe
./nova_benchmark.exe
```

### Python Live Stream Simulation
```bash
pip install pybind11
# Compile the python module
g++ -O3 -Wall -shared -std=c++20 -fPIC \((python3 -m pybind11 --includes) src/bindings.cpp src/OrderBook.cpp -o nova_orderbook\)(python3-config --extension-suffix)
python3 simulate_stream.py
```
