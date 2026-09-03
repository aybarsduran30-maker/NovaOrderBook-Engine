import ctypes
import json
import os
import sys
import time
import redis

dll_dir = os.path.dirname(os.path.abspath(__file__))
if sys.platform == "win32" and hasattr(os, "add_dll_directory"):
    os.add_dll_directory(dll_dir)

dll_path = os.path.join(dll_dir, "nova_engine.dll")
engine = ctypes.CDLL(dll_path)

TRADE_CALLBACK = ctypes.CFUNCTYPE(
    None,
    ctypes.c_uint64,
    ctypes.c_uint64,
    ctypes.c_uint32,
    ctypes.c_uint32,
    ctypes.c_uint64
)

engine.create_order_book.restype = ctypes.c_void_p
engine.destroy_order_book.argtypes = [ctypes.c_void_p]

engine.register_trade_callback.argtypes = [TRADE_CALLBACK]
engine.register_trade_callback.restype = None

engine.add_order.argtypes = [
    ctypes.c_void_p,
    ctypes.c_uint64,
    ctypes.c_bool,
    ctypes.c_uint32,
    ctypes.c_uint32
]
engine.add_order.restype = None

engine.get_best_bid.argtypes = [ctypes.c_void_p]
engine.get_best_bid.restype = ctypes.c_uint32

engine.get_best_ask.argtypes = [ctypes.c_void_p]
engine.get_best_ask.restype = ctypes.c_uint32

r = redis.Redis(host="localhost", port=6379, db=0)

def on_trade(buy_id, sell_id, price, count, timestamp):
    payload = {
        "event": "TRADE",
        "buy_id": buy_id,
        "sell_id": sell_id,
        "price": price,
        "count": count,
        "timestamp_ns": timestamp
    }
    r.xadd("market_trades", {"data": json.dumps(payload)})

c_on_trade = TRADE_CALLBACK(on_trade)
engine.register_trade_callback(c_on_trade)

class NovaEnginePipeline:
    def __init__(self):
        self.book = engine.create_order_book()

    def submit_order(self, order_id: int, is_buy: bool, price: int, count: int):
        engine.add_order(self.book, order_id, is_buy, price, count)
        
        depth_payload = {
            "best_bid": engine.get_best_bid(self.book),
            "best_ask": engine.get_best_ask(self.book),
            "timestamp": time.time_ns()
        }
        r.publish("market_depth", json.dumps(depth_payload))

    def close(self):
        engine.destroy_order_book(self.book)

if __name__ == "__main__":
    pipeline = NovaEnginePipeline()
    
    pipeline.submit_order(101, True, 15000, 100)
    pipeline.submit_order(201, False, 15000, 50)
    pipeline.submit_order(202, False, 15000, 50)
    
    pipeline.close()