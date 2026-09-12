import sys
from pathlib import Path
from typing import List
from contextlib import asynccontextmanager

build_path = Path(__file__).parent / "build" / "Release"
if build_path.exists():
    sys.path.insert(0, str(build_path))

import nova_engine
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from pydantic import BaseModel
import asyncio

class OrderModel(BaseModel):
    id: int
    price: int
    count: int
    side: str
    type: str = "LIMIT"

class BatchRequest(BaseModel):
    orders: List[OrderModel]

engine: nova_engine.OrderBook = None
connected_clients: set[WebSocket] = set()

@asynccontextmanager
async def lifespan(app: FastAPI):
    global engine
    engine = nova_engine.OrderBook()
    broadcast_task = asyncio.create_task(market_data_broadcaster())
    yield
    broadcast_task.cancel()

app = FastAPI(title="NovaOrderBook Ultra-Low Latency Gateway", lifespan=lifespan)

async def market_data_broadcaster():
    while True:
        if connected_clients and engine:
            current_trades = engine.getTradeCount()
            best_bid = engine.getBestBid()
            best_ask = engine.getBestAsk()
            
            payload = {
                "best_bid": best_bid if best_bid != 0 else None,
                "best_ask": best_ask if best_ask != 4294967295 else None,
                "trade_count": current_trades
            }
            
            disconnected = set()
            for ws in connected_clients:
                try:
                    await ws.send_json(payload)
                except Exception:
                    disconnected.add(ws)
            connected_clients.difference_update(disconnected)
            
        await asyncio.sleep(0.01)

@app.post("/api/v1/orders/batch")
async def inject_batch(batch: BatchRequest):
    native_batch = []
    has_order_type = hasattr(nova_engine, "OrderType")
    
    for item in batch.orders:
        side = nova_engine.Side.BUY if item.side.upper() == "BUY" else nova_engine.Side.SELL
        if has_order_type:
            order_type = nova_engine.OrderType.IOC if item.type.upper() == "IOC" else nova_engine.OrderType.LIMIT
            native_batch.append(nova_engine.BatchOrder(item.id, item.price, item.count, side, order_type))
        else:
            native_batch.append(nova_engine.BatchOrder(item.id, item.price, item.count, side))
    
    engine.addOrdersBatch(native_batch)
    return {
        "status": "success",
        "processed_count": len(native_batch),
        "best_bid": engine.getBestBid(),
        "best_ask": engine.getBestAsk(),
        "trade_count": engine.getTradeCount()
    }

@app.websocket("/ws/market")
async def market_ws(websocket: WebSocket):
    await websocket.accept()
    connected_clients.add(websocket)
    try:
        while True:
            await websocket.receive_text()
    except WebSocketDisconnect:
        connected_clients.remove(websocket)