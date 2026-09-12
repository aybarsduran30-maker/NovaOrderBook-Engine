import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

import time
import numpy as np

try:
    import nova_engine
except ImportError:
    raise ImportError("nova_engine module not found.")

WARMUP_ROUNDS = 10_000
BENCHMARK_ROUNDS = 100_000

def run_benchmark():
    book = nova_engine.OrderBook()
    
    side = getattr(nova_engine.Side, 'Buy', getattr(nova_engine.Side, 'Bid', None))
    if side is None:
        side = list(nova_engine.Side.__members__.values())[0]

    for i in range(WARMUP_ROUNDS):
        book.addOrder(i, side, 10000, 10)

    latencies = np.empty(BENCHMARK_ROUNDS, dtype=np.int64)

    for i in range(BENCHMARK_ROUNDS):
        order_id = WARMUP_ROUNDS + i
        price = 10000 + (i % 50)
        qty = 5
        
        start_ns = time.perf_counter_ns()
        book.addOrder(order_id, side, price, qty)
        end_ns = time.perf_counter_ns()
        
        latencies[i] = end_ns - start_ns

    p50 = np.percentile(latencies, 50)
    p90 = np.percentile(latencies, 90)
    p99 = np.percentile(latencies, 99)
    p99_9 = np.percentile(latencies, 99.9)

    print("========================================")
    print(" Pybind11 Overhead + Engine Benchmark   ")
    print("========================================")
    print(f"Samples : {BENCHMARK_ROUNDS:,}")
    print(f"p50     : {p50:.1f} ns")
    print(f"p90     : {p90:.1f} ns")
    print(f"p99     : {p99:.1f} ns")
    print(f"p99.9   : {p99_9:.1f} ns")
    print("========================================")

if __name__ == "__main__":
    run_benchmark()