#pragma once
// =============================================================================
// Momentum trader: "the trend is your friend". Buys rising prices, sells falling ones.
//
// Signal: MOVING AVERAGE CROSSOVER
//   fast MA (last 5 trades) > slow MA (last 20) -> uptrend   -> go long
//   fast MA < slow MA                           -> downtrend -> go short
//
// Momentum bots AMPLIFY moves: they all buy after a rise, which pushes the
// price up more... until it reverses. Fading that overshoot is a classic
// strategy for YOUR trader.
// =============================================================================
#include <deque>
#include <unordered_map>

#include "bots/base_bot.hpp"
#include "common/todo.hpp"

namespace mms {

class Momentum : public BaseBot {
public:
    using BaseBot::BaseBot;

    std::size_t fast_window = 5;
    std::size_t slow_window = 20;
    Qty target_size = 20;  // position to hold while the signal is on
    Qty chunk = 5;         // max qty per order (don't slam the book)

protected:
    void on_trade(const Trade& t) override {
        // TODO(P5): push t.price onto history_[t.symbol]; pop_front while size > slow_window
        not_implemented();
    }

    void on_tick() override {
        // TODO(P5): for each symbol with a FULL slow window:
        //   fast = average of the last fast_window prices; slow = average of all of them
        //   target = fast > slow ? +target_size : -target_size
        //   diff = target - position(s); trade min(|diff|, chunk) toward the target (market orders)
        // TODO(STRETCH): a dead zone (fast must beat slow by X%) to avoid flip-flopping on noise.
        //   The same idea as hysteresis in the LB health checks!
        not_implemented();
    }

private:
    std::unordered_map<Symbol, std::deque<Price>> history_;
};

}  // namespace mms
