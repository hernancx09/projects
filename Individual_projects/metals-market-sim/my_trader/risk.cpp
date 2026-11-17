#include "my_trader/risk.hpp"

#include "common/log.hpp"
#include "common/todo.hpp"

namespace mms {

std::optional<std::string> RiskManager::check(Symbol s, Side side, Qty qty, Qty position, Qty pending_same_side,
                                              Clock::time_point now) {
    // TODO(P6):
    //   if (killed_) return "kill switch";
    //   qty <= 0 or qty > max_order_qty -> reason
    //   worst-case position after fill (include pending_same_side!) beyond +/- max_position -> reason
    //   RATE: pop timestamps older than now - 1s off the front of recent_orders_;
    //         if size >= max_orders_per_sec -> reason; else push_back(now)
    //   return std::nullopt;
    not_implemented();
}

void RiskManager::update_pnl(double pnl_ticks) {
    // TODO(P6): if (!killed_ && pnl_ticks < -limits_.max_loss_ticks) { killed_ = true; log::error("KILL SWITCH ..."); }
    //   After tripping, the trader should cancel everything and flatten (close) its positions.
    not_implemented();
}

}  // namespace mms
