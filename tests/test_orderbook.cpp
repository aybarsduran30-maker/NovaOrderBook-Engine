#include <gtest/gtest.h>
#include "OrderBook.hpp"

class OrderBookTest : public ::testing::Test {
protected:
    OrderBook book;
};

TEST_F(OrderBookTest, AddSingleBuyOrder) {
    book.addOrder(1, Side::BUY, 15000, 100);
    EXPECT_EQ(book.getBestBid(), 15000);
    EXPECT_EQ(book.getTradeCount(), 0);
}

TEST_F(OrderBookTest, AddSingleSellOrder) {
    book.addOrder(1, Side::SELL, 16000, 50);
    EXPECT_EQ(book.getBestAsk(), 16000);
    EXPECT_EQ(book.getTradeCount(), 0);
}

TEST_F(OrderBookTest, FullMatchExecution) {
    book.addOrder(1, Side::SELL, 15000, 100);
    EXPECT_EQ(book.getBestAsk(), 15000);

    book.addOrder(2, Side::BUY, 15000, 100);
    EXPECT_GT(book.getTradeCount(), 0);
}

TEST_F(OrderBookTest, PartialFillExecution) {
    book.addOrder(1, Side::SELL, 15000, 100);
    book.addOrder(2, Side::BUY, 15000, 40);

    EXPECT_GT(book.getTradeCount(), 0);
    EXPECT_EQ(book.getBestAsk(), 15000);
}

TEST_F(OrderBookTest, NoMatchWhenSpreadCrossesOpposite) {
    book.addOrder(1, Side::BUY, 14000, 100);
    book.addOrder(2, Side::SELL, 16000, 100);

    EXPECT_EQ(book.getBestBid(), 14000);
    EXPECT_EQ(book.getBestAsk(), 16000);
    EXPECT_EQ(book.getTradeCount(), 0);
}

TEST_F(OrderBookTest, CancelOrderRemovesLiquidity) {
    book.addOrder(1, Side::BUY, 15000, 100);
    EXPECT_EQ(book.getBestBid(), 15000);

    book.cancelOrder(0);
    book.addOrder(2, Side::SELL, 15000, 50);

    EXPECT_EQ(book.getTradeCount(), 0);
    EXPECT_EQ(book.getBestAsk(), 15000);
}

TEST_F(OrderBookTest, PoolAllocationRecycling) {
    for (uint32_t i = 0; i < 1000; ++i) {
        book.addOrder(i + 1, Side::BUY, 15000, 10);
        book.cancelOrder(0);
    }
    EXPECT_EQ(book.getTradeCount(), 0);
}