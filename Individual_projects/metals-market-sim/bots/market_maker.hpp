#pragma once
// =============================================================================
// Market maker: always offers to BUY a bit below and SELL a bit above the price.
//
//     ask 2400.60   <- MM sells here
//     ----- mid 2400.50 -----
//     bid 2400.40   <- MM buys here
//
// It earns the SPREAD when both sides get hit. Without market makers nothing
// rests on the book, and market orders have nothing to trade against.
// BUILD THIS BOT FIRST.
//
// Risks it must manage:
//   INVENTORY: if everyone sells to it, it piles up a long position and loses
//     when the price drops. Fix: SKEW. When long, shift both quotes DOWN so
//     it's more likely to sell and less likely to buy.
//   ADVERSE SELECTION: informed traders only trade with it when its quote is
//     WRONG compared to fair value. That's the main way market makers lose
//     money, and probably how YOU will beat this bot.
// =============================================================================
#include "bots/base_bot.hpp"
#include "common/todo.hpp"

namespace mms {

class MarketMaker : public BaseBot {
public:
    using BaseBot::BaseBot;  // inherit the constructor

    // Tune these. (TODO(STRETCH): read them from env vars.)
    Price half_spread_ticks = 10;  // quote mid +/- $0.10
    Qty quote_size = 5;
    double skew_per_unit = 1.0;    // ticks to shift per 1 oz of inventory
    Qty max_inventory = 200;

protected:
    void on_tick() override {
        // TODO(P5): for each symbol in kAllSymbols:
        //   reference price: mid(s), else last trade, else a hard-coded start
        //     price (2400.00 / 30.00). SOMEONE has to set the first price when the book is empty!
        //   skew = -position(s) * skew_per_unit
        //   bid = llround(ref - half_spread_ticks + skew); ask = llround(ref + half_spread_ticks + skew)
        //   cancel_all(s); then buy(s, quote_size, bid) / sell(s, quote_size, ask)
        //   skip the side that would push |position| past max_inventory
        //
        // Cancel-and-replace every tick is simple but spammy, and cancels RACE
        //   with fills. TODO(STRETCH): only re-quote when ref moved by more than N ticks.
        // TODO(STRETCH): widen the spread when recent volatility is high.
        not_implemented();
    }
};

}  // namespace mms
