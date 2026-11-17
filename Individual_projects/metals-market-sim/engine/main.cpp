// Entry point for engine_xau / engine_xag.
//   Env: SYMBOL, ORDER_PORT, MD_PORT, ADMIN_PORT
//   Run locally (WSL):  SYMBOL=XAU ./build/bin/engine
#include <exception>
#include <string>

#include "common/config.hpp"
#include "common/event_loop.hpp"
#include "common/log.hpp"
#include "engine/engine.hpp"

int main() {
    try {
        // TODO(P1):
        //   auto sym_name = mms::config::env_str("SYMBOL");
        //   mms::log::init("engine_" + sym_name);
        //   auto symbol = mms::parse_symbol(sym_name);  if (!symbol) throw ...
        //   mms::EventLoop loop;
        //   mms::Engine engine(loop, *symbol);
        //   engine.listen_orders(mms::config::env_int("ORDER_PORT", 9200));
        //   engine.start_market_data(mms::config::env_int("MD_PORT", 9300));    // TODO(P4)
        //   engine.listen_admin(mms::config::env_int("ADMIN_PORT", 9500));      // TODO(P6)
        //   loop.run();
        mms::log::init("engine");
        mms::log::error("engine main() not implemented yet");
        return 1;
    } catch (const std::exception& e) {
        mms::log::error("fatal: ", e.what());
        return 1;
    }
}
