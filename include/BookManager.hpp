#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include "OrderBook.hpp"

class BookManager {
public:
    OrderBook& getOrCreate(const std::string& symbol) {
        auto it = books_.find(symbol);
        if (it == books_.end()) {
            books_[symbol] = std::make_unique<OrderBook>();
        }
        return *books_[symbol];
    }

    bool hasSymbol(const std::string& symbol) const {
        return books_.count(symbol) > 0;
    }

    std::vector<std::string> getSymbols() const {
        std::vector<std::string> result;
        result.reserve(books_.size());
        for (const auto& kv : books_) {
            result.push_back(kv.first);
        }
        return result;
    }

    void removeSymbol(const std::string& symbol) {
        books_.erase(symbol);
    }

private:
    std::unordered_map<std::string, std::unique_ptr<OrderBook>> books_;
};