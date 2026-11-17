#pragma once
// =============================================================================
// YOUR TRADING SYSTEM.
//
// It inherits BaseBot, so it uses the exact same client and hooks as the
// bots. Same rules, same pipes. You win on decisions.
//
// STRATEGY IDEAS (from simplest to most interesting):
//
//  1. BETTER MARKET MAKER. Quote inside the bots' spread so YOU get the fills,
//     and skew inventory smarter. Pull quotes when the informed trader
//     looks active (big one-sided trades).
//
//  2. FADE THE MOMENTUM BOTS. They pile in after a move and overshoot. Detect
//     the overshoot (price far from its moving average) and bet on the reversal.
//
//  3. PAIRS TRADE gold vs silver. Fundamentals are ~0.8 correlated, so the
//     ratio XAU/XAG (~80) is fairly stable. When the MARKET ratio drifts from
//     its recent average by > k standard deviations, sell the expensive one,
//     buy the cheap one, and unwind when it comes back. Neither engine knows
//     about the other symbol, so nobody in this market enforces the ratio. You can.
//
//  4. LATENCY. You get the same market data as the bots. React in on_quote
//     (immediately) instead of on_tick, and you beat them to stale quotes.
//     Then optimize the hot path. This is where C++ pays off.
//
// RULES OF THUMB
//   - Every order goes through risk_.check() first. No exceptions.
//   - Log WHY you traded ("ratio z=2.3, selling XAU"), not just that you did.
//     You'll debug from these lines.
//   - Change ONE thing at a time, with the same SEED, and compare P&L.
// =============================================================================
#include <deque>

#include "bots/base_bot.hpp"
#include "my_trader/risk.hpp"

namespace mms {

class MyTrader : public BaseBot {
public:
    MyTrader(std::string name, EventLoop& loop, std::unique_ptr<ExchangeClient> client, uint64_t seed,
             std::chrono::milliseconds tick, RiskLimits limits);

protected:
    void on_tick() override;
    void on_quote(const Quote& q) override;
    void on_trade(const Trade& t) override;
    void on_fill(const Fill& f) override;
    void on_reject(const Reject& r) override;

private:
    // Wrap buy/sell so EVERY order goes through risk.
    std::optional<ClientOrderId> try_order(Symbol s, Side side, Qty qty, std::optional<Price> limit);
    double mark_to_market_pnl() const;  // your own estimate from fills + current mids

    RiskManager risk_;
    // Your own bookkeeping from fills (don't trust anyone else's numbers):
    int64_t cash_ticks_ = 0;
    // TODO(P6): add the state your strategy needs, e.g. a rolling window of
    //   the XAU/XAG ratio for the pairs trade.
    std::deque<double> ratio_history_;
};

}  // namespace mms
