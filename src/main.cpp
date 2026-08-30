#include <iostream>
#include <iomanip>
#include "OrderBook.hpp"

int main() {
    std::cout << "================================================" << std::endl;
    std::cout << "   NovaOrderBook Engine - High-Frequency Core   " << std::endl;
    std::cout << "================================================" << std::endl;

    OrderBook book;

    std::cout << "\n[1] Submitting Limit Buy Orders..." << std::endl;
    book.addOrder(Order(101, Side::BUY, 150.50, 100));
    book.addOrder(Order(102, Side::BUY, 150.25, 200));
    book.addOrder(Order(103, Side::BUY, 150.75, 50));

    std::cout << "[2] Submitting Limit Sell Orders..." << std::endl;
    book.addOrder(Order(201, Side::SELL, 151.00, 150));
    book.addOrder(Order(202, Side::SELL, 151.25, 300));

    std::cout << "\n--- Market Depth ---" << std::endl;
    std::cout << "Best Bid: $" << std::fixed << std::setprecision(2) << book.getBestBid() << std::endl;
    std::cout << "Best Ask: $" << std::fixed << std::setprecision(2) << book.getBestAsk() << std::endl;

    std::cout << "\n[3] Triggering Execution: Adding Matching Buy Order @ 151.00 (Qty: 200)..." << std::endl;
    book.addOrder(Order(104, Side::BUY, 151.00, 200));

    std::cout << "\n--- Execution History (Trades) ---" << std::endl;
    const auto& trades = book.getTradeHistory();
    for (const auto& t : trades) {
        std::cout << "Trade Executed -> Buy ID: " << t.buyOrderId 
                  << " | Sell ID: " << t.sellOrderId 
                  << " | Price: $" << t.price 
                  << " | Qty: " << t.quantity 
                  << " | Timestamp: " << t.timestamp << " ns" << std::endl;
    }

    std::cout << "\n--- Updated Market Depth ---" << std::endl;
    std::cout << "Best Bid: $" << book.getBestBid() << std::endl;
    std::cout << "Best Ask: $" << book.getBestAsk() << std::endl;

    std::cout << "\n================================================" << std::endl;
    return 0;
}