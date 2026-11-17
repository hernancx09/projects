#include "engine/order_book.hpp"

#include "common/todo.hpp"

namespace mms {

std::vector<Execution> OrderBook::submit(Order& order) {
    // TODO(P3):
    //   std::vector<Execution> out;
    //   if (order.side == Side::Buy) match_against(asks_, order, out);   // buyers hit ASKS
    //   else                         match_against(bids_, order, out);   // sellers hit BIDS
    //   if (order.qty > 0 && order.type == OrderType::Limit) rest(order);
    //   return out;
    not_implemented();
}

template <class Levels>
void OrderBook::match_against(Levels& levels, Order& order, std::vector<Execution>& out) {
    // TODO(P3): THE HEART OF THE EXCHANGE
    //   while (order.qty > 0 && !levels.empty()) {
    //       auto level_it = levels.begin();                 // best price on the other side
    //       Price px = level_it->first;
    //       if (!crosses(order, px)) break;
    //       Level& queue = level_it->second;
    //       while (order.qty > 0 && !queue.empty()) {
    //           Order& resting = queue.front();             // oldest first = TIME priority
    //           Qty q = std::min(order.qty, resting.qty);
    //           order.qty -= q; resting.qty -= q;
    //           out.push_back({resting.id, resting.account, resting.client_order_id, resting.qty, px, q});
    //           if (resting.qty == 0) { index_.erase(resting.id); queue.pop_front(); }
    //       }
    //       if (queue.empty()) levels.erase(level_it);      // no empty levels left behind!
    //   }
    //
    // TODO(P3): SELF-TRADE PREVENTION. If resting.account == order.account,
    //   a bot is trading with itself, which makes fake volume (illegal in real
    //   markets: "wash trading"). Pick a rule (e.g. cancel the resting order)
    //   and add a test for it.
    not_implemented();
}

bool OrderBook::crosses(const Order& incoming, Price resting_price) const {
    // TODO(P3):
    //   Market -> always true
    //   Buy  Limit: resting_price <= incoming.price   (ask is at or below what I'll pay)
    //   Sell Limit: resting_price >= incoming.price   (bid is at or above what I'll accept)
    not_implemented();
}

void OrderBook::rest(const Order& order) {
    // TODO(P3):
    //   auto& level = (order.side == Side::Buy) ? bids_[order.price] : asks_[order.price];
    //     ^ hmm, those are different map types, so a ternary won't compile. Use an if/else.
    //       operator[] creates the level if it doesn't exist. Convenient.
    //   level.push_back(order);
    //   index_[order.id] = Locator{order.side, order.price, std::prev(level.end())};
    not_implemented();
}

bool OrderBook::cancel(OrderId id) {
    // TODO(P3):
    //   auto it = index_.find(id); if not found return false;
    //   look up the level by (side, price), level.erase(locator.it)   <- O(1) thanks to std::list
    //   if the level is now empty, erase it from the map
    //   index_.erase(it); return true;
    not_implemented();
}

std::optional<Price> OrderBook::best_bid() const {
    // TODO(P3): bids_.empty() ? nullopt : bids_.begin()->first
    not_implemented();
}

std::optional<Price> OrderBook::best_ask() const {
    // TODO(P3)
    not_implemented();
}

std::optional<double> OrderBook::mid() const {
    // TODO(P3): needs both sides
    not_implemented();
}

Quote OrderBook::top_of_book() const {
    // TODO(P3): Quote q{symbol_}; for each side, if non-empty: price = begin()->first,
    //   qty = sum of the qty of every order in that level.
    //   (TODO(STRETCH): keep a running total per level instead of summing every time.)
    not_implemented();
}

const Order* OrderBook::find(OrderId id) const {
    // TODO(P3): index_.find -> &*locator.it, or nullptr
    not_implemented();
}

}  // namespace mms
