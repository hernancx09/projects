#pragma once
// =============================================================================
// Informed trader: sees the FAIR VALUE feed. Buys when the market is cheap
// compared to fair value, sells when it's expensive.
//
// This bot PULLS market prices toward the fundamentals. Without it, market
// price and "true" value would drift apart forever. It's also what makes
// market makers lose money (it only trades when their quotes are wrong).
//
// Make it IMPERFECT on purpose, or nobody can ever beat it:
//   - noisy view of fair value
//   - only acts when the mispricing exceeds a threshold
//   - limited size, slow ticks
// =============================================================================
#include <random>

#include "bots/base_bot.hpp"
#include "common/todo.hpp"

namespace mms {

class InformedTrader : public BaseBot {
public:
    using BaseBot::BaseBot;

    Price threshold_ticks = 50;  // act only if the market is > $0.50 away from what it "sees"
    double noise_ticks = 20;     // std-dev of noise on its view of fair value
    Qty max_position = 50;
    Qty clip = 5;

protected:
    void on_tick() override {
        // TODO(P5): for each symbol:
        //   need fair_value_[s] and a two-sided quote, else skip
        //   seen = fair value + std::normal_distribution<double>(0, noise_ticks)(rng_)
        //   if best ASK < seen - threshold and position < max  -> buy clip at the ask (marketable LIMIT)
        //   if best BID > seen + threshold and position > -max -> sell clip at the bid
        //   Why marketable LIMITs instead of market orders? A market order can
        //   "walk the book" to terrible prices if the book is thin.
        not_implemented();
    }
};

}  // namespace mms
