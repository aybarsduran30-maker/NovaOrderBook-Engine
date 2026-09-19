import sys
import time
import json
import os
import redis
from pathlib import Path

build_path = Path(__file__).resolve().parent / "build" / "Debug"
sys.path.insert(0, str(build_path))

import nova_engine

REDIS_URL = os.getenv("REDIS_URL", "redis://localhost:6379/0")
r = redis.Redis.from_url(REDIS_URL, decode_responses=True)

def run_latency_benchmark(rounds=100000, warmup=10000):
    book = nova_engine.OrderBook()

    for i in range(warmup):
        book.addOrder(i, nova_engine.Side.BUY, 10000, 10)

    latencies = []
    for i in range(rounds):
        order_id = warmup + i
        price = 10000 + (i % 50)
        start = time.perf_counter_ns()
        book.addOrder(order_id, nova_engine.Side.BUY, price, 5)
        end = time.perf_counter_ns()
        latencies.append(end - start)

    latencies.sort()

    payload = {
        "samples": rounds,
        "p50": latencies[int(rounds * 0.50)],
        "p90": latencies[int(rounds * 0.90)],
        "p99": latencies[int(rounds * 0.99)],
        "p99_9": latencies[int(rounds * 0.999)],
        "timestamp": time.time()
    }

    r.publish("nova_latency_feed", json.dumps(payload))
    print("Latency published:", payload)
    return payload

if __name__ == "__main__":
    while True:
        run_latency_benchmark()
        time.sleep(30)