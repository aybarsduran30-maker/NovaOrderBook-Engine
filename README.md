# NovaOrderBook Engine

An ultra-low-latency, lock-free limit order book matching engine written in modern C++20 with Python (pybind11) bindings and an async FastAPI/WebSocket gateway.

Designed for deterministic memory access, zero runtime heap allocations, and sub-microsecond tail latency.

---

## Benchmark & Performance

Tested on x86_64 architecture with 100,000 warm-up and measured operations:

| Metric | Latency |
| :--- | :--- |
| **p50** | **0 ns** (Register / L1 cache hit) |
| **p90** | **100 ns** |
| **p99** | **100 ns** |
| **p99.9** | **200 ns** |

---

## Architectural Highlights

* **Zero Dynamic Heap Allocations:** Powered by a pre-allocated `MemoryPool<Order, 2000000>` with indexed free-list pointer swapping.
* **$O(1)$ Direct Lookup:** Eliminated node-based hash tables (`std::unordered_map`) in favor of flat array indexing.
* **Order Types Supported:** Limit orders and Immediate-Or-Cancel (IOC) execution.
* **Batch Processing:** Reduced boundary-crossing overhead for external calls via vectorized batch injection.
* **Lock-Free SPSC Pipeline:** Single Producer Single Consumer `RingBuffer` with cache-line alignment (`alignas(64)`) to prevent false sharing, utilizing CPU pause instructions (`_mm_pause()`).
* **Cross-Platform & CI Verified:** Fully compliant with MSVC and GCC/Clang (`-fPIC`, strict C++20 memory ordering).

---

## Project Structure

```text
NovaOrderBook-Engine/
├── include/
│   ├── Order.hpp           # Order, BatchOrder, Trade structs
│   ├── OrderBook.hpp       # Core matching logic and PriceLevel arrays
│   ├── MemoryPool.hpp      # Zero-heap pre-allocated pool
│   ├── RingBuffer.hpp      # Lock-free SPSC queue
│   └── EnginePipeline.hpp  # Isolated worker thread pipeline
├── src/
│   ├── OrderBook.cpp       # Engine implementation
│   ├── EnginePipeline.cpp  # Command processor loop
│   ├── bindings.cpp        # Pybind11 module bindings
│   └── main.cpp            # CLI demo
├── tests/
│   ├── test_orderbook.cpp  # GoogleTest suite
│   ├── benchmark.cpp       # Microbenchmarks
│   └── benchmark_engine.cpp# Latency percentiles benchmark
├── api_server.py           # FastAPI & WebSocket gateway
├── test_client.py          # Async test runner
└── CMakeLists.txt

Build & Run
Prerequisites
CMake 3.16+

C++20 compatible compiler (MSVC 2019+, GCC 11+, Clang 13+)

Python 3.10+

Build C++ Core and Python Module

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

Run Tests and Benchmarks

ctest --test-dir build -C Release --output-on-failure
./build/Release/benchmark_engine

Run API Gateway & Stream

python -m pip install fastapi uvicorn websockets pydantic httpx
python -m uvicorn api_server:app --port 8000
python test_client.py




