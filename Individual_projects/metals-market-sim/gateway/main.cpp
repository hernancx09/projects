// Entry point for each gateway container.
//   Env: GATEWAY_ID, LISTEN_PORT, ENGINES, RATE_LIMIT_PER_SEC
//   Run locally (WSL):  GATEWAY_ID=gw1 ENGINES=XAU=localhost:9200 ./build/bin/gateway
#include <exception>

#include "common/config.hpp"
#include "common/event_loop.hpp"
#include "common/log.hpp"
#include "gateway/gateway.hpp"

int main() {
    try {
        // TODO(P2):
        //   auto id = mms::config::env_str("GATEWAY_ID");
        //   mms::log::init("gateway_" + id);
        //   mms::EventLoop loop;
        //   mms::Gateway gw(loop, id, mms::config::parse_symbol_map(mms::config::env_str("ENGINES")),
        //                   mms::config::env_double("RATE_LIMIT_PER_SEC", 50));
        //   gw.connect_engines();
        //   gw.listen(mms::config::env_int("LISTEN_PORT", 9100));
        //   loop.run();
        mms::log::init("gateway");
        mms::log::error("gateway main() not implemented yet");
        return 1;
    } catch (const std::exception& e) {
        mms::log::error("fatal: ", e.what());
        return 1;
    }
}
