import json
import redis

r = redis.Redis(host="localhost", port=6379, db=0)

class AlphaMetricsConsumer:
    def __init__(self):
        self.trades = []

    def start(self):
        print("Listening for trades from NovaOrderBook...")
        last_id = "0"
        while True:
            events = r.xread({"market_trades": last_id}, count=10, block=1000)
            if not events:
                continue

            for stream_name, messages in events:
                for msg_id, data in messages:
                    trade = json.loads(data[b"data"].decode("utf-8"))
                    self.trades.append(trade)
                    last_id = msg_id

                    recent = self.trades[-100:]
                    vol = sum(t["count"] for t in recent)
                    turnover = sum(t["price"] * t["count"] for t in recent)
                    vwap = turnover / vol if vol > 0 else 0

                    print(f"[AlphaMetrics] Match -> Price: {trade['price']} | Qty: {trade['count']} | Rolling VWAP: {vwap:.2f}")

if __name__ == "__main__":
    consumer = AlphaMetricsConsumer()
    consumer.start()