import os
import sys
import time
import random
import json
from pathlib import Path
import redis

root_dir = Path(__file__).resolve().parent
release_path = root_dir / "build" / "Release"
build_path = root_dir / "build"

sys.path.insert(0, str(release_path))
sys.path.insert(0, str(build_path))
sys.path.insert(0, str(root_dir))

import nova_engine

REDIS_URL = os.getenv("REDIS_URL", "redis://localhost:6379/0")
r = redis.Redis.from_url(REDIS_URL, decode_responses=True)

def run():
    book = nova_engine.OrderBook()
    order_id = 1000
    mid_price = 15000
    spread_range = 4

    last_trades = 0
    total_buy_vol = 0
    total_sell_vol = 0

    try:
        while True:
            order_id += 1
            is_buy = random.random() < 0.52
            side = nova_engine.Side.BUY if is_buy else nova_engine.Side.SELL
            
            if side == nova_engine.Side.BUY:
                price = random.randint(mid_price - 1, mid_price + spread_range)
                qty = random.randint(10, 100)
                total_buy_vol += qty
            else:
                price = random.randint(mid_price - spread_range, mid_price + 1)
                qty = random.randint(10, 100)
                total_sell_vol += qty

            book.addOrder(order_id, side, price, qty)

            raw_bid = book.getBestBid()
            raw_ask = book.getBestAsk()

            best_bid = raw_bid if (0 < raw_bid < 1000000) else 0
            best_ask = raw_ask if (0 < raw_ask < 1000000) else 0

            if best_bid > 0 and best_ask > 0 and best_ask >= best_bid:
                spread = best_ask - best_bid
            else:
                spread = 0

            current_trades = book.getTradeCount()
            if current_trades > last_trades:
                ofi = (total_buy_vol - total_sell_vol) / (total_buy_vol + total_sell_vol + 1e-6)
                
                trade_payload = {
                    "event": "TRADE",
                    "buy_id": order_id if side == nova_engine.Side.BUY else 0,
                    "sell_id": order_id if side == nova_engine.Side.SELL else 0,
                    "price": price,
                    "count": qty,
                    "best_bid": best_bid,
                    "best_ask": best_ask,
                    "spread": spread,
                    "ofi": round(ofi, 4),
                    "timestamp_ns": time.time_ns()
                }
                r.xadd("market_trades", {"data": json.dumps(trade_payload)})
                last_trades = current_trades

            time.sleep(0.01)
    except KeyboardInterrupt:
        pass

if __name__ == "__main__":
    run()