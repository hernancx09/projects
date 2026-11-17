#pragma once
// =============================================================================
// Accounts: who owns what, for ONE symbol. (Each engine has its own.)
//
// Per account:
//   position  ounces held (+ long, - short)
//   cash      change in cash from trading THIS symbol, in ticks
//             buy 2 oz @ 240000 -> cash -= 480000
//
//   P&L (mark-to-market) = cash + position * current_price
//   e.g. buy 2 @ 2400.00, price now 2410.00: -4800 + 2*2410 = +$20
//
// THE DISTRIBUTED SYSTEMS PROBLEM HIDING HERE
// -------------------------------------------
// Gold and silver live on DIFFERENT engines (shards). An account's "total
// cash" is split across two processes, so neither engine can check "can you
// afford this?" against the TOTAL. Options:
//   a) per-symbol limits only (this scaffold: simple, a bit unrealistic)
//   b) a central risk service every order checks with (one more network hop = more
//      latency, plus a single point of failure)
//   c) give each engine a pre-allocated slice of each account's buying power
// Real exchanges and brokers deal with exactly this. Write down what you chose and why.
//
// tools/leaderboard.cpp adds the two engines' numbers together.
// =============================================================================
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

#include "common/messages.hpp"

namespace mms {

struct AccountState {
    Qty position = 0;
    int64_t cash = 0;     // ticks (int64: 1000 oz * $3000 * 100 ticks fits easily; int32 would NOT)
    int64_t volume = 0;   // total qty traded
    int64_t trades = 0;
    Qty open_buy_qty = 0;   // resting buy orders, for the risk check
    Qty open_sell_qty = 0;
};

class Accounts {
public:
    static constexpr Qty kMaxPosition = 1000;  // per symbol, long or short
    static constexpr Qty kMaxOrderQty = 100;   // TODO(P3): tune. Different for bots vs you? Is that fair?

    explicit Accounts(Symbol symbol) : symbol_(symbol) {}

    AccountState& get(const std::string& account);  // creates on first use

    // Pre-trade risk check: would this order, FULLY filled, together with
    // existing open orders on the same side, push |position| past kMaxPosition?
    std::optional<RejectReason> check_order(const std::string& account, Side side, Qty qty) const;

    void apply_fill(const std::string& account, Side side, Price price, Qty qty);

    // Mark-to-market P&L in ticks. With no mark price, value the position at 0
    // (or better: at the last trade price. Your choice).
    double pnl(const std::string& account, std::optional<double> mark) const;

    // One line per account for the admin endpoint:
    //   "<account> <position> <cash> <pnl> <volume> <trades>\n"
    // Plain text is easy to read with `nc` and easy to parse in leaderboard.cpp.
    std::string snapshot(std::optional<double> mark) const;

    const std::unordered_map<std::string, AccountState>& all() const { return accounts_; }

private:
    Symbol symbol_;
    std::unordered_map<std::string, AccountState> accounts_;
};

}  // namespace mms
