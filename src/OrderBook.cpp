#include "OrderBook.hpp"
#include <chrono>

typedef void (*TradeCallback)(uint64_t buyId, uint64_t sellId, uint32_t price, uint32_t count, uint64_t timestamp);
static TradeCallback g_tradeCallback = nullptr;

OrderBook::OrderBook() 
    : poolHead(0), 
      bestBidPrice(0), 
      bestAskPrice(UINT32_MAX), 
      tradeCount(0) {
    orderPool.resize(MAX_ORDERS);
}

uint32_t OrderBook::allocateOrder() {
    return poolHead++;
}

void OrderBook::freeOrder(uint32_t index) {
    orderPool[index].count = 0;
}

void OrderBook::cancelOrder(uint32_t orderPoolIndex) {
    if (orderPoolIndex >= poolHead) return;

    Order& order = orderPool[orderPoolIndex];
    if (order.count == 0) return;

    uint32_t levelIdx = order.price - MIN_PRICE;
    auto& book = (order.side == Side::BUY) ? bids : asks;
    PriceLevel& level = book[levelIdx];

    if (order.prevOrderIndex != NULL_INDEX) {
        orderPool[order.prevOrderIndex].nextOrderIndex = order.nextOrderIndex;
    } else {
        level.headIndex = order.nextOrderIndex;
    }

    if (order.nextOrderIndex != NULL_INDEX) {
        orderPool[order.nextOrderIndex].prevOrderIndex = order.prevOrderIndex;
    } else {
        level.tailIndex = order.prevOrderIndex;
    }

    level.totalVolume -= order.count;
    freeOrder(orderPoolIndex);

    if (level.headIndex == NULL_INDEX) {
        if (order.side == Side::BUY) {
            if (order.price == bestBidPrice) {
                uint32_t nextBid = 0;
                for (int32_t p = static_cast<int32_t>(levelIdx) - 1; p >= 0; --p) {
                    if (bids[p].headIndex != NULL_INDEX) {
                        nextBid = static_cast<uint32_t>(p) + MIN_PRICE;
                        break;
                    }
                }
                bestBidPrice = nextBid;
            }
        } else {
            if (order.price == bestAskPrice) {
                uint32_t nextAsk = UINT32_MAX;
                for (uint32_t p = levelIdx + 1; p < PRICE_RANGE; ++p) {
                    if (asks[p].headIndex != NULL_INDEX) {
                        nextAsk = p + MIN_PRICE;
                        break;
                    }
                }
                bestAskPrice = nextAsk;
            }
        }
    }
}

void OrderBook::match(Side incomingSide, uint32_t incomingPrice, uint32_t& incomingCount, uint64_t incomingId) {
    auto now = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());

    if (incomingSide == Side::BUY) {
        while (incomingCount > 0 && bestAskPrice <= incomingPrice) {
            uint32_t levelIdx = bestAskPrice - MIN_PRICE;
            PriceLevel& level = asks[levelIdx];
            uint32_t currIdx = level.headIndex;

            while (currIdx != NULL_INDEX && incomingCount > 0) {
                Order& resting = orderPool[currIdx];
                uint32_t matchQty = (incomingCount < resting.count) ? incomingCount : resting.count;

                resting.count -= matchQty;
                incomingCount -= matchQty;
                level.totalVolume -= matchQty;
                tradeCount++;

                if (g_tradeCallback) {
                    g_tradeCallback(incomingId, resting.id, resting.price, matchQty, now);
                }

                if (resting.count == 0) {
                    uint32_t next = resting.nextOrderIndex;
                    level.headIndex = next;
                    if (next != NULL_INDEX) {
                        orderPool[next].prevOrderIndex = NULL_INDEX;
                    } else {
                        level.tailIndex = NULL_INDEX;
                    }
                    freeOrder(currIdx);
                    currIdx = next;
                }
            }

            if (level.headIndex == NULL_INDEX) {
                uint32_t nextAsk = UINT32_MAX;
                for (uint32_t p = levelIdx + 1; p < PRICE_RANGE; ++p) {
                    if (asks[p].headIndex != NULL_INDEX) {
                        nextAsk = p + MIN_PRICE;
                        break;
                    }
                }
                bestAskPrice = nextAsk;
            }
        }
    } else {
        while (incomingCount > 0 && bestBidPrice >= incomingPrice && bestBidPrice != 0) {
            uint32_t levelIdx = bestBidPrice - MIN_PRICE;
            PriceLevel& level = bids[levelIdx];
            uint32_t currIdx = level.headIndex;

            while (currIdx != NULL_INDEX && incomingCount > 0) {
                Order& resting = orderPool[currIdx];
                uint32_t matchQty = (incomingCount < resting.count) ? incomingCount : resting.count;

                resting.count -= matchQty;
                incomingCount -= matchQty;
                level.totalVolume -= matchQty;
                tradeCount++;

                if (g_tradeCallback) {
                    g_tradeCallback(resting.id, incomingId, resting.price, matchQty, now);
                }

                if (resting.count == 0) {
                    uint32_t next = resting.nextOrderIndex;
                    level.headIndex = next;
                    if (next != NULL_INDEX) {
                        orderPool[next].prevOrderIndex = NULL_INDEX;
                    } else {
                        level.tailIndex = NULL_INDEX;
                    }
                    freeOrder(currIdx);
                    currIdx = next;
                }
            }

            if (level.headIndex == NULL_INDEX) {
                uint32_t nextBid = 0;
                for (int32_t p = static_cast<int32_t>(levelIdx) - 1; p >= 0; --p) {
                    if (bids[p].headIndex != NULL_INDEX) {
                        nextBid = static_cast<uint32_t>(p) + MIN_PRICE;
                        break;
                    }
                }
                bestBidPrice = nextBid;
            }
        }
    }
}

void OrderBook::addOrder(uint64_t id, Side side, uint32_t price, uint32_t count) {
    if (price < MIN_PRICE || price > MAX_PRICE || count == 0) return;

    match(side, price, count, id);

    if (count == 0) return;

    uint32_t orderIdx = allocateOrder();
    orderPool[orderIdx] = {id, price, count, side, NULL_INDEX, NULL_INDEX};

    uint32_t levelIdx = price - MIN_PRICE;
    auto& book = (side == Side::BUY) ? bids : asks;
    PriceLevel& level = book[levelIdx];

    if (level.tailIndex == NULL_INDEX) {
        level.headIndex = orderIdx;
        level.tailIndex = orderIdx;
    } else {
        orderPool[level.tailIndex].nextOrderIndex = orderIdx;
        orderPool[orderIdx].prevOrderIndex = level.tailIndex;
        level.tailIndex = orderIdx;
    }
    level.totalVolume += count;

    if (side == Side::BUY) {
        if (price > bestBidPrice) bestBidPrice = price;
    } else {
        if (price < bestAskPrice) bestAskPrice = price;
    }
}

extern "C" {
    OrderBook* create_order_book() {
        return new OrderBook();
    }

    void destroy_order_book(OrderBook* book) {
        delete book;
    }

    void register_trade_callback(TradeCallback cb) {
        g_tradeCallback = cb;
    }

    void add_order(OrderBook* book, uint64_t orderId, bool isBuy, uint32_t price, uint32_t count) {
        Side side = isBuy ? Side::BUY : Side::SELL;
        book->addOrder(orderId, side, price, count);
    }

    void cancel_order(OrderBook* book, uint32_t orderPoolIndex) {
        book->cancelOrder(orderPoolIndex);
    }

    uint32_t get_best_bid(OrderBook* book) {
        return book->getBestBid();
    }

    uint32_t get_best_ask(OrderBook* book) {
        return book->getBestAsk();
    }

    uint64_t get_trade_count(OrderBook* book) {
        return book->getTradeCount();
    }
}