#include "OrderBook.hpp"
#include <chrono>
#include <cstdio>
#include <algorithm>

typedef void (*TradeCallback)(uint64_t buyId, uint64_t sellId, uint32_t price, uint32_t count, uint64_t timestamp);
static TradeCallback g_tradeCallback = nullptr;

OrderBook::OrderBook() 
    : bestBidPrice(0), 
      bestAskPrice(UINT32_MAX), 
      tradeCount(0) {
    orderLookup.assign(MAX_ORDERS, NULL_INDEX);
}

void OrderBook::cancelOrder(uint32_t orderPoolIndex) {
    Order& order = pool[orderPoolIndex];
    if (order.count == 0) return;

    uint32_t levelIdx = order.price - MIN_PRICE;
    auto& book = (order.side == Side::BUY) ? bids : asks;
    PriceLevel& level = book[levelIdx];

    if (order.prevOrderIndex != NULL_INDEX) {
        pool[order.prevOrderIndex].nextOrderIndex = order.nextOrderIndex;
    } else {
        level.headIndex = order.nextOrderIndex;
    }

    if (order.nextOrderIndex != NULL_INDEX) {
        pool[order.nextOrderIndex].prevOrderIndex = order.prevOrderIndex;
    } else {
        level.tailIndex = order.prevOrderIndex;
    }

    level.totalVolume -= order.count;
    if (order.id < orderLookup.size()) {
        orderLookup[order.id] = NULL_INDEX;
    }
    pool.deallocate(orderPoolIndex);

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

bool OrderBook::cancelOrderById(uint64_t orderId) {
    if (orderId >= orderLookup.size()) return false;
    uint32_t poolIdx = orderLookup[orderId];
    if (poolIdx == NULL_INDEX) return false;
    cancelOrder(poolIdx);
    return true;
}

void OrderBook::match(Side incomingSide, uint32_t incomingPrice, uint32_t& incomingCount, uint64_t incomingId) {
    uint64_t now = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());

    if (incomingSide == Side::BUY) {
        while (incomingCount > 0 && bestAskPrice <= incomingPrice) {
            uint32_t levelIdx = bestAskPrice - MIN_PRICE;
            PriceLevel& level = asks[levelIdx];
            uint32_t currIdx = level.headIndex;

            while (currIdx != NULL_INDEX && incomingCount > 0) {
                Order& resting = pool[currIdx];
                uint32_t matchQty = (incomingCount < resting.count) ? incomingCount : resting.count;

                resting.count -= matchQty;
                incomingCount -= matchQty;
                level.totalVolume -= matchQty;
                tradeCount++;
                ofiBuyVolume += matchQty;

                if (tradeCallback) {
                    Trade t{incomingId, resting.id, resting.price, matchQty, now};
                    tradeCallback(t);
                }
                if (g_tradeCallback) {
                    g_tradeCallback(incomingId, resting.id, resting.price, matchQty, now);
                }

                if (resting.count == 0) {
                    uint32_t next = resting.nextOrderIndex;
                    level.headIndex = next;
                    if (next != NULL_INDEX) {
                        pool[next].prevOrderIndex = NULL_INDEX;
                    } else {
                        level.tailIndex = NULL_INDEX;
                    }
                    if (resting.id < orderLookup.size()) {
                        orderLookup[resting.id] = NULL_INDEX;
                    }
                    pool.deallocate(currIdx);
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
                Order& resting = pool[currIdx];
                uint32_t matchQty = (incomingCount < resting.count) ? incomingCount : resting.count;

                resting.count -= matchQty;
                incomingCount -= matchQty;
                level.totalVolume -= matchQty;
                tradeCount++;
                ofiSellVolume += matchQty;

                if (tradeCallback) {
                    Trade t{resting.id, incomingId, resting.price, matchQty, now};
                    tradeCallback(t);
                }
                if (g_tradeCallback) {
                    g_tradeCallback(resting.id, incomingId, resting.price, matchQty, now);
                }

                if (resting.count == 0) {
                    uint32_t next = resting.nextOrderIndex;
                    level.headIndex = next;
                    if (next != NULL_INDEX) {
                        pool[next].prevOrderIndex = NULL_INDEX;
                    } else {
                        level.tailIndex = NULL_INDEX;
                    }
                    if (resting.id < orderLookup.size()) {
                        orderLookup[resting.id] = NULL_INDEX;
                    }
                    pool.deallocate(currIdx);
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

void OrderBook::addOrder(uint64_t id, Side side, uint32_t price, uint32_t count, OrderType type) {
    if (price < MIN_PRICE || price > MAX_PRICE || count == 0) return;

    match(side, price, count, id);
    if (count == 0 || type == OrderType::IOC) return;
    if (type == OrderType::MARKET) return;

    uint32_t orderIdx = pool.allocate();
    pool[orderIdx] = {id, price, count, side, NULL_INDEX, NULL_INDEX};
    if (id < orderLookup.size()) {
        orderLookup[id] = orderIdx;
    }

    uint32_t levelIdx = price - MIN_PRICE;
    auto& book = (side == Side::BUY) ? bids : asks;
    PriceLevel& level = book[levelIdx];

    if (level.tailIndex == NULL_INDEX) {
        level.headIndex = orderIdx;
        level.tailIndex = orderIdx;
    } else {
        pool[level.tailIndex].nextOrderIndex = orderIdx;
        pool[orderIdx].prevOrderIndex = level.tailIndex;
        level.tailIndex = orderIdx;
    }
    level.totalVolume += count;

    if (side == Side::BUY) {
        if (price > bestBidPrice) bestBidPrice = price;
    } else {
        if (price < bestAskPrice) bestAskPrice = price;
    }
}

void OrderBook::addOrdersBatch(const std::vector<BatchOrder>& orders) {
    for (const auto& ord : orders) {
        addOrder(ord.id, ord.side, ord.price, ord.count, ord.type);
    }
}

std::vector<OrderBook::LevelSnapshot> OrderBook::getTopBids(size_t n) const {
    std::vector<LevelSnapshot> result;
    result.reserve(n);
    if (bestBidPrice < MIN_PRICE) return result;

    for (int32_t p = static_cast<int32_t>(bestBidPrice - MIN_PRICE);
         p >= 0 && result.size() < n; --p) {
        if (bids[p].headIndex != NULL_INDEX) {
            result.push_back({static_cast<uint32_t>(p) + MIN_PRICE, bids[p].totalVolume});
        }
    }
    return result;
}

std::vector<OrderBook::LevelSnapshot> OrderBook::getTopAsks(size_t n) const {
    std::vector<LevelSnapshot> result;
    result.reserve(n);
    if (bestAskPrice > MAX_PRICE) return result;

    for (uint32_t p = bestAskPrice - MIN_PRICE;
         p < PRICE_RANGE && result.size() < n; ++p) {
        if (asks[p].headIndex != NULL_INDEX) {
            result.push_back({p + MIN_PRICE, asks[p].totalVolume});
        }
    }
    return result;
}

bool OrderBook::amendOrder(uint64_t orderId, uint32_t newPrice, uint32_t newCount) {
    if (orderId >= orderLookup.size()) return false;
    uint32_t poolIdx = orderLookup[orderId];
    if (poolIdx == NULL_INDEX) return false;

    Side side = pool[poolIdx].side;
    cancelOrder(poolIdx);

    if (newPrice < MIN_PRICE || newPrice > MAX_PRICE || newCount == 0) return false;

    uint32_t orderIdx = pool.allocate();
    pool[orderIdx] = {orderId, newPrice, newCount, side, NULL_INDEX, NULL_INDEX};
    orderLookup[orderId] = orderIdx;

    uint32_t levelIdx = newPrice - MIN_PRICE;
    auto& book = (side == Side::BUY) ? bids : asks;
    PriceLevel& level = book[levelIdx];

    if (level.tailIndex == NULL_INDEX) {
        level.headIndex = orderIdx;
        level.tailIndex = orderIdx;
    } else {
        pool[level.tailIndex].nextOrderIndex = orderIdx;
        pool[orderIdx].prevOrderIndex = level.tailIndex;
        level.tailIndex = orderIdx;
    }
    level.totalVolume += newCount;

    if (side == Side::BUY) {
        if (newPrice > bestBidPrice) bestBidPrice = newPrice;
    } else {
        if (newPrice < bestAskPrice) bestAskPrice = newPrice;
    }

    return true;
}

bool OrderBook::saveSnapshot(const std::string& filepath) const {
    FILE* f = fopen(filepath.c_str(), "wb");
    if (!f) return false;

    fwrite(&bestBidPrice, sizeof(bestBidPrice), 1, f);
    fwrite(&bestAskPrice, sizeof(bestAskPrice), 1, f);
    fwrite(&tradeCount, sizeof(tradeCount), 1, f);
    fwrite(&ofiBuyVolume, sizeof(ofiBuyVolume), 1, f);
    fwrite(&ofiSellVolume, sizeof(ofiSellVolume), 1, f);

    for (uint32_t p = 0; p < PRICE_RANGE; ++p) {
        uint32_t idx = bids[p].headIndex;
        while (idx != NULL_INDEX) {
            const Order& o = pool[idx];
            fwrite(&o.id, sizeof(o.id), 1, f);
            fwrite(&o.price, sizeof(o.price), 1, f);
            fwrite(&o.count, sizeof(o.count), 1, f);
            uint8_t side = static_cast<uint8_t>(o.side);
            fwrite(&side, sizeof(side), 1, f);
            idx = o.nextOrderIndex;
        }
    }

    for (uint32_t p = 0; p < PRICE_RANGE; ++p) {
        uint32_t idx = asks[p].headIndex;
        while (idx != NULL_INDEX) {
            const Order& o = pool[idx];
            fwrite(&o.id, sizeof(o.id), 1, f);
            fwrite(&o.price, sizeof(o.price), 1, f);
            fwrite(&o.count, sizeof(o.count), 1, f);
            uint8_t side = static_cast<uint8_t>(o.side);
            fwrite(&side, sizeof(side), 1, f);
            idx = o.nextOrderIndex;
        }
    }

    uint64_t sentinel = 0xFFFFFFFFFFFFFFFF;
    fwrite(&sentinel, sizeof(sentinel), 1, f);
    fclose(f);
    return true;
}

bool OrderBook::loadSnapshot(const std::string& filepath) {
    FILE* f = fopen(filepath.c_str(), "rb");
    if (!f) return false;

    bids.fill(PriceLevel{});
    asks.fill(PriceLevel{});
    pool.reset();
    std::fill(orderLookup.begin(), orderLookup.end(), NULL_INDEX);

    fread(&bestBidPrice, sizeof(bestBidPrice), 1, f);
    fread(&bestAskPrice, sizeof(bestAskPrice), 1, f);
    fread(&tradeCount, sizeof(tradeCount), 1, f);
    fread(&ofiBuyVolume, sizeof(ofiBuyVolume), 1, f);
    fread(&ofiSellVolume, sizeof(ofiSellVolume), 1, f);

    uint64_t id;
    while (fread(&id, sizeof(id), 1, f) == 1) {
        if (id == 0xFFFFFFFFFFFFFFFF) break;

        uint32_t price, count;
        uint8_t side;
        fread(&price, sizeof(price), 1, f);
        fread(&count, sizeof(count), 1, f);
        fread(&side, sizeof(side), 1, f);

        uint32_t orderIdx = pool.allocate();
        pool[orderIdx] = {id, price, count, static_cast<Side>(side), NULL_INDEX, NULL_INDEX};
        if (id < orderLookup.size()) orderLookup[id] = orderIdx;

        uint32_t levelIdx = price - MIN_PRICE;
        auto& book = (static_cast<Side>(side) == Side::BUY) ? bids : asks;
        PriceLevel& level = book[levelIdx];

        if (level.tailIndex == NULL_INDEX) {
            level.headIndex = orderIdx;
            level.tailIndex = orderIdx;
        } else {
            pool[level.tailIndex].nextOrderIndex = orderIdx;
            pool[orderIdx].prevOrderIndex = level.tailIndex;
            level.tailIndex = orderIdx;
        }
        level.totalVolume += count;
    }

    fclose(f);
    return true;
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
        book->addOrder(orderId, side, price, count, OrderType::LIMIT);
    }

    void add_order_ioc(OrderBook* book, uint64_t orderId, bool isBuy, uint32_t price, uint32_t count) {
        Side side = isBuy ? Side::BUY : Side::SELL;
        book->addOrder(orderId, side, price, count, OrderType::IOC);
    }

    void cancel_order(OrderBook* book, uint32_t orderPoolIndex) {
        book->cancelOrder(orderPoolIndex);
    }

    bool cancel_order_by_id(OrderBook* book, uint64_t orderId) {
        return book->cancelOrderById(orderId);
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