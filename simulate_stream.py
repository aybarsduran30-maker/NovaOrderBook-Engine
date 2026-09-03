import sys
import time
import os
import random
from pathlib import Path

release_path = Path(__file__).resolve().parent / "build" / "Release"
sys.path.append(str(release_path))

import nova_engine

def render_book(book, step, total_steps):
    os.system("cls" if os.name == "nt" else "clear")
    best_bid = book.getBestBid()
    best_ask = book.getBestAsk()
    trades = book.getTradeCount()
    
    spread = (best_ask - best_bid) if (best_ask != 0 and best_bid != 0) else 0

    print("=" * 45)
    print(f"       NOVA LOB - LIVE TERMINAL DEPTH       ")
    print("=" * 45)
    print(f" Events Processed : {step} / {total_steps}")
    print(f" Total Executions : {trades}")
    print(f" Current Spread   : {spread} ticks")
    print("-" * 45)
    print(" SIDE      | PRICE      | DEPTH")
    print("-" * 45)

    if best_ask != 0:
        for offset in range(4, -1, -1):
            price = best_ask + offset
            depth_bar = "#" * random.randint(3, 18)
            print(f" \033[91mASK (SELL)\033[0m | {price:<10} | {depth_bar}")
    else:
        print(" \033[91mASK (SELL)\033[0m | EMPTY      | -")

    print("-" * 45)
    print(f" -----> SPREAD: [{best_bid} - {best_ask}] <-----")
    print("-" * 45)

    if best_bid != 0:
        for offset in range(5):
            price = best_bid - offset
            depth_bar = "#" * random.randint(3, 18)
            print(f" \033[92mBID (BUY) \033[0m | {price:<10} | {depth_bar}")
    else:
        print(" \033[92mBID (BUY) \033[0m | EMPTY      | -")

    print("=" * 45)

def run_live_feed(total_orders=500, delay_sec=0.03):
    book = nova_engine.OrderBook()
    mid_price = 15000
    spread_range = 8

    for i in range(1, total_orders + 1):
        side = nova_engine.Side.BUY if random.random() < 0.5 else nova_engine.Side.SELL
        
        if side == nova_engine.Side.BUY:
            price = random.randint(mid_price - spread_range, mid_price + 2)
        else:
            price = random.randint(mid_price - 2, mid_price + spread_range)

        qty = random.randint(5, 50)
        book.addOrder(i, side, price, qty)

        if i % 3 == 0:
            render_book(book, i, total_orders)
            time.sleep(delay_sec)

    render_book(book, total_orders, total_orders)
    print("\nLive simulation stream ended.")

if __name__ == "__main__":
    run_live_feed()