#pragma once
#include <thread>
#include <atomic>
#include <vector>
#include <cstdint>
#include "RingBuffer.hpp"
#include "OrderBook.hpp"

enum class CommandType : uint8_t {
    ADD_ORDER = 0,
    CANCEL_ORDER = 1
};

struct PipelineCommand {
    CommandType type;
    uint64_t id;
    Side side;
    uint32_t price;
    uint32_t count;
    OrderType orderType;
};

class EnginePipeline {
public:
    EnginePipeline();
    ~EnginePipeline();

    void start();
    void stop();

    bool submitOrder(uint64_t id, Side side, uint32_t price, uint32_t count, OrderType type = OrderType::LIMIT);
    bool cancelOrder(uint64_t id);

    OrderBook& getBook() { return book; }

private:
    OrderBook book;
    RingBuffer<PipelineCommand, 65536> incomingQueue;
    std::atomic<bool> running{false};
    std::thread workerThread;

    void processLoop();
};