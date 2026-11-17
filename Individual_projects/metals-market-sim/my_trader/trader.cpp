#include "my_trader/trader.hpp"

#include "common/log.hpp"
#include "common/todo.hpp"

namespace mms {

MyTrader::MyTrader(std::string name, EventLoop& loop, std::unique_ptr<ExchangeClient> client, uint64_t seed,
                   std::chrono::milliseconds tick, RiskLimits limits)
    : BaseBot(std::move(name), loop, std::move(client), seed, tick), risk_(limits) {}

void MyTrader::on_tick() {
    // TODO(P6): risk_.update_pnl(mark_to_market_pnl());
    //   if (risk_.killed()) { cancel_all(); flatten positions; return; }
    // TODO(P6): YOUR STRATEGY. Start simple (idea 1 or 2 in trader.hpp), make
    //   sure it's not losing money, THEN get clever.
    not_implemented();
}

void MyTrader::on_quote(const Quote& q) {
    // TODO(P6, optional): react instantly to quote changes (idea 4).
    //   Measure your reaction time: protocol::now_ns() here vs. when you send the order.
}

void MyTrader::on_trade(const Trade& t) {
    // TODO(P6, optional): watch the tape. Big aggressive trades = someone knows something?
}

void MyTrader::on_fill(const Fill& f) {
    // TODO(P6): update cash_ticks_ (BaseBot already updates position_)
    //   log::info("FILL ", f.side, " ", f.qty, " ", f.symbol, " @ ", f.price);
    not_implemented();
}

void MyTrader::on_reject(const Reject& r) {
    // TODO(P6): rejects mean your view of the world is wrong, or you're breaking a rule. Log them loudly.
}

std::optional<ClientOrderId> MyTrader::try_order(Symbol s, Side side, Qty qty, std::optional<Price> limit) {
    // TODO(P6):
    //   Qty pending = sum of remaining qty of open orders with this symbol + side
    //   if (auto why = risk_.check(s, side, qty, position(s), pending)) { log::warn("risk blocked: ", *why); return nullopt; }
    //   return side == Side::Buy ? buy(s, qty, limit) : sell(s, qty, limit);
    not_implemented();
}

double MyTrader::mark_to_market_pnl() const {
    // TODO(P6): cash_ticks_ + sum over symbols of position(s) * mid(s)
    not_implemented();
}

}  // namespace mms
