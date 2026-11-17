// Entry point for the load balancer container.
//   Env: LISTEN_PORT, BACKENDS, STRATEGY, HEALTH_CHECK_INTERVAL_MS, STATS_PORT
//   Run locally (WSL):  BACKENDS=localhost:9100 STRATEGY=round_robin ./build/bin/load_balancer
#include <exception>

#include "common/config.hpp"
#include "common/event_loop.hpp"
#include "common/log.hpp"
#include "load_balancer/health.hpp"
#include "load_balancer/load_balancer.hpp"
#include "load_balancer/strategies.hpp"

int main() {
    mms::log::init("load_balancer");
    try {
        // TODO(P2):
        //   mms::lb::BackendList backends;
        //   for (auto& hp : mms::config::parse_addr_list(mms::config::env_str("BACKENDS")))
        //       backends.push_back(std::make_unique<mms::lb::Backend>()); ...set host/port
        //   auto strategy = mms::lb::make_strategy(mms::config::env_str("STRATEGY", "round_robin"));
        //
        //   mms::EventLoop loop;
        //   mms::lb::LoadBalancer lb(loop, backends, std::move(strategy));
        //   lb.listen(mms::config::env_int("LISTEN_PORT", 9000));
        //
        //   mms::lb::HealthChecker health(backends, std::chrono::milliseconds(mms::config::env_int("HEALTH_CHECK_INTERVAL_MS", 2000)));
        //   health.start();
        //
        //   loop.run();
        mms::log::error("load_balancer main() not implemented yet");
        return 1;
    } catch (const std::exception& e) {
        // Exiting with an error makes `docker compose ps` show it as exited(1). Easy to spot.
        mms::log::error("fatal: ", e.what());
        return 1;
    }
}
