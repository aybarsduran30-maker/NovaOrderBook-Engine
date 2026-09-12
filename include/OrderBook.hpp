#pragma once
#include <array>
#include <vector>
#include <cstdint>
#include "Order.hpp"
#include "MemoryPool.hpp"

class OrderBook {
public:
    static constexpr uint32_t MIN_PRICE = 10000;
    static constexpr uint32_t MAX_PRICE = 20000;
    static constexpr uint32_t PRICE_RANGE = MAX_PRICE - MIN_PRICE + 1;
    static constexpr uint32_t MAX_ORDERS = 2000000;
    static constexpr uint32_t NULL_INDEX = 0xFFFFFFFF;

    struct PriceLevel {
        uint32_t headIndex{NULL_INDEX};
        uint32_t tailIndex{NULL_INDEX};
        uint32_t totalVolume{0};
    };

    OrderBook();

    void addOrder(uint64_t id, Side side, uint32_t price, uint32_t count, OrderType type = OrderType::LIMIT);
    void addOrdersBatch(const std::vector<BatchOrder>& orders);
    bool cancelOrderById(uint64_t orderId);
    void cancelOrder(uint32_t orderPoolIndex);

    uint32_t getBestBid() const { return bestBidPrice; }
    uint32_t getBestAsk() const { return bestAskPrice; }
    uint64_t getTradeCount() const { return tradeCount; }

private:
    std::array<PriceLevel, PRICE_RANGE> bids;
    std::array<PriceLevel, PRICE_RANGE> asks;
    MemoryPool<Order, MAX_ORDERS> pool;
    std::vector<uint32_t> orderLookup;

    uint32_t bestBidPrice;
    uint32_t bestAskPrice;
    uint64_t tradeCount;

    void match(Side incomingSide, uint32_t incomingPrice, uint32_t& incomingCount, uint64_t incomingId);
};