#pragma once
#include <cstdint>

enum class Side : uint8_t {
    BUY = 0,
    SELL = 1
};

struct Order {
    uint64_t id;
    uint32_t price;
    uint32_t count;
    Side side;
    uint32_t prevOrderIndex;
    uint32_t nextOrderIndex;
};

struct Trade {
    uint64_t buyOrderId;
    uint64_t sellOrderId;
    uint32_t price;
    uint32_t count;
    uint64_t timestamp;
};