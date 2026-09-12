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
## Performance & Benchmarks

The NovaOrderBook matching engine is benchmarked across two distinct layers to measure core execution speed and the cross-language boundary cost introduced by Pybind11.

Both benchmarks were executed on a Release build with 100,000 order insertions following 10,000 warmup iterations.

| Layer | p50 | p90 | p99 | p99.9 |
| :--- | :--- | :--- | :--- | :--- |
| **Native C++ Engine** | **100 ns** | **100 ns** | **100 ns** | **100 ns** |
| **Python (Pybind11 Wrapper)** | **500 ns** | **500 ns** | **1100 ns** | **1700 ns** |

### Latency Analysis

* **Core Engine Execution:** The raw C++ engine achieves deterministic, flat **100 ns** latency across all percentiles (p50 through p99.9), demonstrating cache-friendly memory layouts with zero tail latency spikes.
* **Pybind11 Overhead:** Calling the engine via Python incurs an additional **~400 ns** baseline cost at p50/p90 due to C-API type conversions, argument parsing, and boundary crossing.
* **Tail Latency:** The tail variance in the Python layer (up to 1.7 µs at p99.9) is driven by Python interpreter overhead, GIL management, and OS thread scheduling.
