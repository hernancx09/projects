#pragma once
// =============================================================================
// Limit order book + matching for ONE symbol. Pure logic, no networking.
// Tests: tests/test_order_book.cpp
//
// WHAT A BOOK LOOKS LIKE
//             price    qty   (orders at that price, OLDEST first)
//   ASKS    2401.00     7    [#12: 5, #15: 2]      <- sellers. Lowest = BEST ask
//           2400.50     3    [#9: 3]
//   ------------------------------------------ spread = 0.50
//   BIDS    2400.00    10    [#3: 4, #8: 6]        <- buyers. Highest = BEST bid
//           2399.50     2    [#4: 2]
//
// PRICE-TIME PRIORITY: an incoming order trades against the BEST price
// first. At the same price, OLDEST first (FIFO). So each price level is a queue.
//
// EXAMPLE: BUY LIMIT 8 @ 2401.00 arrives
//   best ask 2400.50 <= 2401.00 -> trade 3 with #9 @ 2400.50 (level now empty -> remove)
//   best ask 2401.00 <= 2401.00 -> trade 5 with #12 @ 2401.00 (#12 done -> remove)
//   remaining 0 -> done. (Anything left over would REST as a bid @ 2401.00.)
// The trade price is ALWAYS the RESTING order's price. The aggressor gets
// any "price improvement".
//
// MARKET orders behave like a limit with an infinitely aggressive price. Any
// unfilled remainder is DROPPED (never rests).
//
// DATA STRUCTURES (this is where C++ gets interesting)
//   std::map<Price, Level>  : sorted tree. begin() is the best price.
//                             bids use std::greater (highest first), asks std::less (lowest first).
//   Level = std::list<Order>: FIFO queue. Why list, not deque? std::list
//                             iterators stay valid when OTHER elements are
//                             added or removed. So we store an iterator per
//                             order in index_ and cancel in O(1).
//   index_                  : order id -> (side, price, iterator) for cancels.
//
// FASTER (TODO(STRETCH), and what real HFT engines do):
//   - prices live in a narrow band, so use a flat ARRAY indexed by
//     (price - min_price): O(1) level lookup, cache friendly
//   - intrusive linked lists + an object pool: zero allocation while matching
//   Benchmark before and after with a few million random orders.
// =============================================================================
#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/messages.hpp"

namespace mms {

struct Order {
    OrderId id = 0;
    std::string account;
    ClientOrderId client_order_id = 0;
    Side side = Side::Buy;
    OrderType type = OrderType::Limit;
    Price price = 0;    // ignored for Market
    Qty qty = 0;        // REMAINING quantity (decreases as it fills)
    Qty original_qty = 0;
};

// One match between the incoming (aggressor) order and a resting order.
// Copies of the resting order's fields, because the resting order may be
// erased from the book by the time the engine processes this.
struct Execution {
    OrderId resting_id = 0;
    std::string resting_account;
    ClientOrderId resting_client_order_id = 0;
    Qty resting_remaining = 0;   // after this execution
    Price price = 0;             // = the resting order's price
    Qty qty = 0;
};

class OrderBook {
public:
    explicit OrderBook(Symbol symbol) : symbol_(symbol) {}

    OrderId next_order_id() { return next_id_++; }

    // Match `order` against the book. Returns the executions in the order they
    // happened. Afterwards `order.qty` is what's left. For Limit orders that
    // remainder now RESTS in the book. For Market orders it was dropped.
    std::vector<Execution> submit(Order& order);

    // Remove a resting order. false if not found (already filled or never existed).
    bool cancel(OrderId id);

    std::optional<Price> best_bid() const;
    std::optional<Price> best_ask() const;
    std::optional<double> mid() const;  // (bid + ask) / 2, needs both sides
    Quote top_of_book() const;          // best level of each side, qty = total at that level

    const Order* find(OrderId id) const;  // nullptr if not resting
    std::size_t order_count() const { return index_.size(); }
    Symbol symbol() const { return symbol_; }

private:
    using Level = std::list<Order>;
    using BidLevels = std::map<Price, Level, std::greater<Price>>;  // begin() = highest bid
    using AskLevels = std::map<Price, Level, std::less<Price>>;     // begin() = lowest ask

    struct Locator {
        Side side;
        Price price;
        Level::iterator it;
    };

    // Shared matching loop. A template, because BidLevels and AskLevels are
    // DIFFERENT types (different comparators) that share the same interface.
    template <class Levels>
    void match_against(Levels& levels, Order& order, std::vector<Execution>& out);

    bool crosses(const Order& incoming, Price resting_price) const;
    void rest(const Order& order);

    Symbol symbol_;
    BidLevels bids_;
    AskLevels asks_;
    std::unordered_map<OrderId, Locator> index_;
    OrderId next_id_ = 1;
};

}  // namespace mms
