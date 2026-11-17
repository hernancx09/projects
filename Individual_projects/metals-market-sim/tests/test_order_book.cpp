// =============================================================================
// Order book tests: the rules of the market, written as code.
// They FAIL until you implement engine/order_book.cpp.
//
// TODO(P3): add tests for self-trade prevention once you pick a rule, and for
//   cancelling an order in the MIDDLE of a level's queue.
// =============================================================================
#include <gtest/gtest.h>

#include "engine/order_book.hpp"

using namespace mms;

namespace {

Order limit(OrderBook& book, const std::string& acct, Side side, Price px, Qty qty) {
    Order o;
    o.id = book.next_order_id();
    o.account = acct;
    o.client_order_id = o.id;
    o.side = side;
    o.type = OrderType::Limit;
    o.price = px;
    o.qty = o.original_qty = qty;
    return o;
}

Order market(OrderBook& book, const std::string& acct, Side side, Qty qty) {
    Order o = limit(book, acct, side, 0, qty);
    o.type = OrderType::Market;
    return o;
}

}  // namespace

TEST(OrderBook, EmptyBook) {
    OrderBook book(Symbol::XAU);
    EXPECT_FALSE(book.best_bid().has_value());
    EXPECT_FALSE(book.best_ask().has_value());
    EXPECT_FALSE(book.mid().has_value());
}

TEST(OrderBook, NonCrossingOrdersRest) {
    OrderBook book(Symbol::XAU);
    auto b = limit(book, "a", Side::Buy, 100, 5);
    auto s = limit(book, "b", Side::Sell, 102, 3);
    EXPECT_TRUE(book.submit(b).empty());
    EXPECT_TRUE(book.submit(s).empty());
    EXPECT_EQ(book.best_bid(), 100);
    EXPECT_EQ(book.best_ask(), 102);
    EXPECT_DOUBLE_EQ(*book.mid(), 101.0);

    Quote q = book.top_of_book();
    EXPECT_EQ(q.bid_qty, 5);
    EXPECT_EQ(q.ask_qty, 3);
}

TEST(OrderBook, BestBidIsHighest) {
    OrderBook book(Symbol::XAU);
    for (Price px : {99, 101, 100}) {
        auto o = limit(book, "a", Side::Buy, px, 1);
        book.submit(o);
    }
    EXPECT_EQ(book.best_bid(), 101);
}

TEST(OrderBook, CrossingTradesAtRestingPrice) {
    OrderBook book(Symbol::XAU);
    auto ask = limit(book, "seller", Side::Sell, 100, 5);
    book.submit(ask);

    auto buy = limit(book, "buyer", Side::Buy, 105, 5);  // willing to pay 105...
    auto ex = book.submit(buy);
    ASSERT_EQ(ex.size(), 1u);
    EXPECT_EQ(ex[0].price, 100);  // ...but trades at the RESTING price
    EXPECT_EQ(ex[0].qty, 5);
    EXPECT_EQ(ex[0].resting_account, "seller");
    EXPECT_EQ(buy.qty, 0);
    EXPECT_EQ(book.order_count(), 0u);
}

TEST(OrderBook, TimePriorityAtSamePrice) {
    OrderBook book(Symbol::XAU);
    auto first = limit(book, "first", Side::Sell, 100, 2);
    auto second = limit(book, "second", Side::Sell, 100, 2);
    book.submit(first);
    book.submit(second);

    auto buy = limit(book, "buyer", Side::Buy, 100, 3);
    auto ex = book.submit(buy);
    ASSERT_EQ(ex.size(), 2u);
    EXPECT_EQ(ex[0].resting_account, "first");  // oldest first
    EXPECT_EQ(ex[0].qty, 2);
    EXPECT_EQ(ex[1].resting_account, "second");
    EXPECT_EQ(ex[1].qty, 1);
    EXPECT_EQ(ex[1].resting_remaining, 1);
}

TEST(OrderBook, WalksTheBookAcrossLevels) {
    OrderBook book(Symbol::XAU);
    auto a1 = limit(book, "s", Side::Sell, 101, 1);
    auto a2 = limit(book, "s", Side::Sell, 102, 1);
    auto a3 = limit(book, "s", Side::Sell, 103, 1);
    book.submit(a1);
    book.submit(a2);
    book.submit(a3);

    auto buy = limit(book, "b", Side::Buy, 102, 5);
    auto ex = book.submit(buy);
    ASSERT_EQ(ex.size(), 2u);  // 101 and 102, but NOT 103 (above the limit)
    EXPECT_EQ(ex[0].price, 101);
    EXPECT_EQ(ex[1].price, 102);
    EXPECT_EQ(buy.qty, 3);              // remainder...
    EXPECT_EQ(book.best_bid(), 102);    // ...rests as the new best bid
    EXPECT_EQ(book.best_ask(), 103);
}

TEST(OrderBook, MarketOrderNeverRests) {
    OrderBook book(Symbol::XAU);
    auto ask = limit(book, "s", Side::Sell, 100, 2);
    book.submit(ask);

    auto mkt = market(book, "b", Side::Buy, 5);
    auto ex = book.submit(mkt);
    ASSERT_EQ(ex.size(), 1u);
    EXPECT_EQ(ex[0].qty, 2);
    EXPECT_EQ(mkt.qty, 3);                     // 3 unfilled...
    EXPECT_FALSE(book.best_bid().has_value()); // ...and NOT resting
}

TEST(OrderBook, CancelRemovesOrderAndEmptyLevel) {
    OrderBook book(Symbol::XAU);
    auto b = limit(book, "a", Side::Buy, 100, 5);
    book.submit(b);
    EXPECT_TRUE(book.cancel(b.id));
    EXPECT_FALSE(book.best_bid().has_value());  // no empty level left behind
    EXPECT_FALSE(book.cancel(b.id));            // second cancel: not found
}

TEST(OrderBook, FilledOrderCannotBeCancelled) {
    OrderBook book(Symbol::XAU);
    auto ask = limit(book, "s", Side::Sell, 100, 1);
    book.submit(ask);
    auto buy = limit(book, "b", Side::Buy, 100, 1);
    book.submit(buy);
    EXPECT_FALSE(book.cancel(ask.id));
}
