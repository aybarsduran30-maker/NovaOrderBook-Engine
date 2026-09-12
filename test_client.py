import asyncio
import httpx
import websockets
import json

async def listen_ws():
    uri = "ws://127.0.0.1:8000/ws/market"
    async with websockets.connect(uri) as ws:
        for _ in range(5):
            msg = await ws.recv()
            print(f"[WS Data] {msg}")

async def send_orders():
    await asyncio.sleep(0.5)
    async with httpx.AsyncClient() as client:
        payload = {
            "orders": [
                {"id": 101, "price": 10500, "count": 10, "side": "BUY", "type": "LIMIT"},
                {"id": 102, "price": 10600, "count": 5, "side": "SELL", "type": "LIMIT"},
                {"id": 103, "price": 10600, "count": 5, "side": "BUY", "type": "IOC"}
            ]
        }
        resp = await client.post("http://127.0.0.1:8000/api/v1/orders/batch", json=payload)
        print(f"[REST Status] {resp.status_code}")
        print(f"[REST Body] {resp.text}")

async def main():
    await asyncio.gather(listen_ws(), send_orders())

if __name__ == "__main__":
    asyncio.run(main())