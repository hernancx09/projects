#pragma once
// =============================================================================
// Noise trader: buys or sells randomly, for no reason related to price.
//
// Sounds useless, but noise traders are the market's background activity.
// They create volume, pay the market maker's spread, and keep prices
// wobbling. In real markets they're people trading for reasons that have
// nothing to do with price (they need cash, they're rebalancing, ...).
//
// Over time they should LOSE money (they pay the spread). If they're winning,
// something's off.
// =============================================================================
#include <random>

#include "bots/base_bot.hpp"
#include "common/todo.hpp"

namespace mms {

class NoiseTrader : public BaseBot {
public:
    using BaseBot::BaseBot;

    double trade_probability = 0.3;  // chance of trading on each tick
    Qty max_qty = 3;
    Qty max_abs_position = 30;

protected:
    void on_tick() override {
        // TODO(P5):
        //   std::uniform_real_distribution<double> u(0, 1); if (u(rng_) > trade_probability) return;
        //   pick a random symbol and side (std::uniform_int_distribution), qty in [1, max_qty]
        //   if |position| > max_abs_position, force the side that REDUCES it
        //   mostly market orders; sometimes a limit a few ticks from mid (adds book depth)
        not_implemented();
    }
};

}  // namespace mms
