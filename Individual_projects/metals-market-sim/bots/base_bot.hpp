#pragma once
// =============================================================================
// BaseBot: the skeleton every bot (and YOUR trader) is built on.
//
// Owns an ExchangeClient and keeps a LOCAL VIEW of the world built from
// incoming messages: latest quotes, last trades, fair values (if subscribed),
// and our own positions.
//
// Subclasses override the virtual hooks they care about:
//   on_tick()        every `tick` interval. Most logic goes here.
//   on_quote(q)      top of book changed
//   on_trade(t)      someone traded
//   on_fill(f)       WE traded
//   on_fair_value(v) fundamentals update
//
// EVENT-DRIVEN (on_quote) vs TIMER-DRIVEN (on_tick): reacting to events is
// fast, but can spam orders on every tiny change. Ticks are calmer. Real
// strategies use both. Start with on_tick.
//
// YOUR VIEW IS ALWAYS STALE. By the time you act on a quote, the book may
// have changed. That's latency, and it's why orders sometimes don't fill or
// fill at a different price than you expected.
// =============================================================================
#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>

#include "common/client.hpp"
#include "common/event_loop.hpp"
#include "common/messages.hpp"

namespace mms {

class BaseBot {
public:
    BaseBot(std::string name, EventLoop& loop, std::unique_ptr<ExchangeClient> client, uint64_t seed,
            std::chrono::milliseconds tick);
    virtual ~BaseBot() = default;  // VIRTUAL destructor: we delete bots through BaseBot pointers

    // Wire callbacks, connect, subscribe, start the tick timer.
    void start();

    const std::string& name() const { return name_; }

protected:
    // ---- hooks: override these ----
    virtual void on_tick() {}
    virtual void on_quote(const Quote&) {}
    virtual void on_trade(const Trade&) {}
    virtual void on_fill(const Fill&) {}
    virtual void on_reject(const Reject&) {}
    virtual void on_fair_value(const FairValue&) {}

    // ---- helpers for subclasses ----
    std::optional<double> mid(Symbol s) const;  // needs a two-sided quote
    Qty position(Symbol s) const;
    ClientOrderId buy(Symbol s, Qty qty, std::optional<Price> limit = std::nullopt);   // no limit = market
    ClientOrderId sell(Symbol s, Qty qty, std::optional<Price> limit = std::nullopt);
    void cancel_all();
    void cancel_all(Symbol s);

    std::string name_;
    EventLoop& loop_;
    std::unique_ptr<ExchangeClient> client_;
    std::mt19937_64 rng_;  // each bot gets its own seeded RNG -> reproducible runs
    std::chrono::milliseconds tick_;

    std::unordered_map<Symbol, Quote> quotes_;
    std::unordered_map<Symbol, Price> last_trade_;
    std::unordered_map<Symbol, Price> fair_value_;
    std::unordered_map<Symbol, Qty> position_;

private:
    void wire_callbacks();  // client_->on_quote = [this](const Quote& q) { ...update state...; on_quote(q); }; etc.
};

}  // namespace mms
