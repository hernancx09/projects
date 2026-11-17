#include "engine/accounts.hpp"

#include <sstream>

#include "common/todo.hpp"

namespace mms {

AccountState& Accounts::get(const std::string& account) {
    // TODO(P3): return accounts_[account];   (operator[] default-constructs if missing)
    not_implemented();
}

std::optional<RejectReason> Accounts::check_order(const std::string& account, Side side, Qty qty) const {
    // TODO(P3):
    //   qty > kMaxOrderQty -> RiskLimit
    //   look up the account (missing = all zeros. Don't use operator[] in a const method: it won't compile!)
    //   worst case for a BUY : position + open_buy_qty  + qty > kMaxPosition  -> RiskLimit
    //   worst case for a SELL: position - open_sell_qty - qty < -kMaxPosition -> RiskLimit
    //   else nullopt
    // Remember to keep open_buy_qty/open_sell_qty updated as orders rest, fill, and cancel
    //   (that bookkeeping happens in engine.cpp).
    not_implemented();
}

void Accounts::apply_fill(const std::string& account, Side side, Price price, Qty qty) {
    // TODO(P3):
    //   BUY : position += qty; cash -= price * qty
    //   SELL: position -= qty; cash += price * qty
    //   volume += qty; ++trades
    //   Watch the types: price (int64) * qty (int32) -> int64. Good. But Qty*Qty stays int32...
    not_implemented();
}

double Accounts::pnl(const std::string& account, std::optional<double> mark) const {
    // TODO(P3): cash + position * mark
    not_implemented();
}

std::string Accounts::snapshot(std::optional<double> mark) const {
    // TODO(P6): std::ostringstream; one line per account (see the header)
    not_implemented();
}

}  // namespace mms
