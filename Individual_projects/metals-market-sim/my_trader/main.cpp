// Entry point for YOUR trader's container.
//   Env: ACCOUNT, LB_ADDR, MD_ADDRS, (FUNDAMENTALS_ADDR: see docker-compose.yml), SEED
#include <exception>

#include "common/config.hpp"
#include "common/event_loop.hpp"
#include "common/log.hpp"
#include "my_trader/trader.hpp"

int main() {
    mms::log::init("my_trader");
    try {
        // TODO(P6):
        //   mms::EventLoop loop;
        //   auto client = std::make_unique<mms::ExchangeClient>(loop, mms::config::env_str("ACCOUNT", "me"),
        //       mms::config::parse_addr(mms::config::env_str("LB_ADDR")),
        //       mms::config::parse_symbol_map(mms::config::env_str("MD_ADDRS")));
        //   mms::MyTrader trader("me", loop, std::move(client), seed, std::chrono::milliseconds(250), mms::RiskLimits{});
        //   trader.start();
        //   loop.run();
        mms::log::error("my_trader main() not implemented yet");
        return 1;
    } catch (const std::exception& e) {
        mms::log::error("fatal: ", e.what());
        return 1;
    }
}
