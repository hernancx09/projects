#pragma once
// =============================================================================
// YOUR risk manager. BUILD THIS BEFORE YOUR STRATEGY.
//
// Every order your strategy wants goes through check() first.
//   strategy = what you WANT to do
//   risk     = what you're ALLOWED to do
// Keep them separate, so a bug in the strategy can't blow up the account.
// (Knight Capital lost $440M in 45 minutes in 2012 to a strategy bug that
// risk controls should have stopped. Worth reading about.)
// =============================================================================
#include <chrono>
#include <cstdint>
#include <deque>
#include <optional>
#include <string>

#include "common/messages.hpp"

namespace mms {

struct RiskLimits {
    Qty max_position = 100;          // per symbol
    Qty max_order_qty = 20;
    int max_orders_per_sec = 10;
    double max_loss_ticks = 500'000; // $5,000. Below this, the kill switch trips
};

class RiskManager {
public:
    using Clock = std::chrono::steady_clock;

    explicit RiskManager(RiskLimits limits) : limits_(limits) {}

    // nullopt = allowed; otherwise a human-readable reason (log it!).
    // `pending_same_side` = qty already in OPEN orders on this side. Without it,
    // 5 open buy orders can each pass alone and together break the limit.
    std::optional<std::string> check(Symbol s, Side side, Qty qty, Qty position, Qty pending_same_side,
                                     Clock::time_point now = Clock::now());

    // Call regularly with your mark-to-market P&L (in ticks).
    void update_pnl(double pnl_ticks);

    bool killed() const { return killed_; }

private:
    RiskLimits limits_;
    bool killed_ = false;
    std::deque<Clock::time_point> recent_orders_;  // timestamps from the last second (sliding window)
};

}  // namespace mms
