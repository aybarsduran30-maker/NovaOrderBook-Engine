#include <iostream>
#include <iomanip>
#include "OrderBook.hpp"

int main() {
    std::cout << "================================================" << std::endl;
    std::cout << "   NovaOrderBook Engine - High-Frequency Core   " << std::endl;
    std::cout << "================================================" << std::endl;

    OrderBook book;

    std::cout << "\n[1] Submitting Limit Buy Orders..." << std::endl;
    book.addOrder(101, Side::BUY, 15050, 100);
    book.addOrder(102, Side::BUY, 15025, 200);
    book.addOrder(103, Side::BUY, 15075, 50);

    std::cout << "[2] Submitting Limit Sell Orders..." << std::endl;
    book.addOrder(201, Side::SELL, 15100, 150);
    book.addOrder(202, Side::SELL, 15125, 300);

    std::cout << "\n--- Market Depth ---" << std::endl;
    std::cout << "Best Bid: " << book.getBestBid() << std::endl;
    std::cout << "Best Ask: " << book.getBestAsk() << std::endl;

    std::cout << "\n[3] Triggering Execution: Adding Matching Buy Order @ 15100 (Qty: 200)..." << std::endl;
    book.addOrder(104, Side::BUY, 15100, 200);

    std::cout << "\n--- Execution History (Trades) ---" << std::endl;
    std::cout << "Total Trades Executed: " << book.getTradeCount() << std::endl;

    std::cout << "\n--- Updated Market Depth ---" << std::endl;
    std::cout << "Best Bid: " << book.getBestBid() << std::endl;
    std::cout << "Best Ask: " << book.getBestAsk() << std::endl;

    std::cout << "\n================================================" << std::endl;
    return 0;
}