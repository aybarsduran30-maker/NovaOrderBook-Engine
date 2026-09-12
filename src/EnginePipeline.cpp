#include "EnginePipeline.hpp"

EnginePipeline::EnginePipeline() = default;

EnginePipeline::~EnginePipeline() {
    stop();
}

void EnginePipeline::start() {
    if (running.exchange(true)) return;

    workerThread = std::thread(&EnginePipeline::processLoop, this);
}

void EnginePipeline::stop() {
    if (!running.exchange(false)) return;

    if (workerThread.joinable()) {
        workerThread.join();
    }
}

bool EnginePipeline::submitOrder(uint64_t id, Side side, uint32_t price, uint32_t count, OrderType type) {
    PipelineCommand cmd{CommandType::ADD_ORDER, id, side, price, count, type};
    return incomingQueue.push(cmd);
}

bool EnginePipeline::cancelOrder(uint64_t id) {
    PipelineCommand cmd{CommandType::CANCEL_ORDER, id, Side::BUY, 0, 0, OrderType::LIMIT};
    return incomingQueue.push(cmd);
}

void EnginePipeline::processLoop() {
    PipelineCommand cmd;

    while (running.load(std::memory_order_relaxed)) {
        if (incomingQueue.pop(cmd)) {
            if (cmd.type == CommandType::ADD_ORDER) {
                book.addOrder(cmd.id, cmd.side, cmd.price, cmd.count, cmd.orderType);
            } else if (cmd.type == CommandType::CANCEL_ORDER) {
                book.cancelOrderById(cmd.id);
            }
        } else {
            HARDWARE_PAUSE();
        }
    }

    while (incomingQueue.pop(cmd)) {
        if (cmd.type == CommandType::ADD_ORDER) {
            book.addOrder(cmd.id, cmd.side, cmd.price, cmd.count, cmd.orderType);
        } else if (cmd.type == CommandType::CANCEL_ORDER) {
            book.cancelOrderById(cmd.id);
        }
    }
}