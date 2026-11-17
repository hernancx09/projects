#include "bots/base_bot.hpp"

#include "common/log.hpp"
#include "common/todo.hpp"

namespace mms {

BaseBot::BaseBot(std::string name, EventLoop& loop, std::unique_ptr<ExchangeClient> client, uint64_t seed,
                 std::chrono::milliseconds tick)
    : name_(std::move(name)), loop_(loop), client_(std::move(client)), rng_(seed), tick_(tick) {}

void BaseBot::start() {
    // TODO(P5):
    //   wire_callbacks();
    //   client_->connect();
    //   client_->subscribe();
    //   loop_.add_timer(tick_, [this] {
    //       if (!client_->logged_in()) return;
    //       try { on_tick(); }
    //       catch (const std::exception& e) { log::error(name_, " tick failed: ", e.what()); }
    //         ^ a bug in one bot's tick shouldn't kill the other 9 bots in this process
    //   });
    // TODO(STRETCH): stagger start times so 10 bots don't all tick at the same instant.
    not_implemented();
}

void BaseBot::wire_callbacks() {
    // TODO(P5): for each client_ callback, update local state, then call the virtual hook:
    //   client_->on_quote = [this](const Quote& q) { quotes_[q.symbol] = q; on_quote(q); };
    //   client_->on_trade -> last_trade_
    //   client_->on_fill  -> position_ (+qty for Buy, -qty for Sell)
    //   client_->on_fair_value -> fair_value_[XAU], fair_value_[XAG]
    //   client_->on_reject -> log::warn + on_reject
    //   client_->on_gap -> log::warn (watch these when netem is on!)
    not_implemented();
}

std::optional<double> BaseBot::mid(Symbol s) const {
    // TODO(P5): find the quote; need bid_qty > 0 AND ask_qty > 0; return (bid + ask) / 2.0
    not_implemented();
}

Qty BaseBot::position(Symbol s) const {
    // TODO(P5): find in position_, or 0
    not_implemented();
}

ClientOrderId BaseBot::buy(Symbol s, Qty qty, std::optional<Price> limit) {
    // TODO(P5): client_->send_order(s, Side::Buy, limit ? OrderType::Limit : OrderType::Market, qty, limit.value_or(0));
    not_implemented();
}

ClientOrderId BaseBot::sell(Symbol s, Qty qty, std::optional<Price> limit) {
    // TODO(P5)
    not_implemented();
}

void BaseBot::cancel_all() {
    // TODO(P5): collect ids from client_->open_orders() FIRST, then cancel each.
    //   (Don't modify a map while iterating over it.)
    not_implemented();
}

void BaseBot::cancel_all(Symbol s) {
    // TODO(P5): same, filtered by symbol
    not_implemented();
}

}  // namespace mms
