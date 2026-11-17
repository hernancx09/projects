#pragma once
// =============================================================================
// Load balancing strategies: which backend gets this NEW connection?
//
// Our LB balances CONNECTIONS, not messages. Once a client is connected to
// gateway2, everything it sends goes to gateway2 until it disconnects.
// That's how an L4 (TCP-level) load balancer works: it never looks inside
// the bytes, so it CAN'T route per message.
//
//   L4: sees IPs, ports, TCP. Fast and protocol-agnostic. (what we build)
//   L7: parses the application protocol (HTTP, or our frames) and can route
//       each request separately. Smarter, slower.
//   TODO(STRETCH): an L7 mode that routes each NewOrder independently. What
//     breaks? (Hint: replies for one client now come back through different gateways.)
//
// This file has NO socket code, so it's unit-testable: tests/test_strategies.cpp
// =============================================================================
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace mms::lb {

struct Backend {
    std::string host;
    uint16_t port = 0;

    // Written by the health-check THREAD, read by the event-loop thread.
    // std::atomic makes that safe without a mutex. A plain bool shared
    // between threads is a data race = undefined behavior.
    std::atomic<bool> healthy{true};

    // Only touched by the event-loop thread -> plain ints are fine.
    int active_connections = 0;
    uint64_t total_connections = 0;

    // Only touched by the health-check thread.
    int consecutive_failures = 0;
    int consecutive_successes = 0;

    std::string name() const { return host + ":" + std::to_string(port); }
};

// std::atomic can't be copied or moved, so neither can Backend. We hold them
// by unique_ptr: the Backend objects never move in memory, and both threads
// can keep raw pointers to them safely.
using BackendList = std::vector<std::unique_ptr<Backend>>;

class Strategy {
public:
    virtual ~Strategy() = default;
    // Return a HEALTHY backend, or nullptr if none.
    virtual Backend* pick(const BackendList& backends, std::string_view client_ip) = 0;
};

// 1, 2, 3, 1, 2, 3, ... skipping unhealthy backends.
// Fair if every connection costs the same. Ours don't: a market maker sends
// 100x more orders than a noise trader.
class RoundRobin : public Strategy {
public:
    Backend* pick(const BackendList& backends, std::string_view client_ip) override;

private:
    std::size_t next_ = 0;
};

// The healthy backend with the fewest active connections.
class LeastConnections : public Strategy {
public:
    Backend* pick(const BackendList& backends, std::string_view client_ip) override;
};

// Same client IP -> same backend ("sticky"). Useful when backends cache per-client state.
class IpHash : public Strategy {
public:
    Backend* pick(const BackendList& backends, std::string_view client_ip) override;
};

// TODO(STRETCH): class ConsistentHash : public Strategy  (a hash ring with virtual nodes)

// "round_robin" | "least_connections" | "ip_hash"  -> throws std::invalid_argument otherwise
std::unique_ptr<Strategy> make_strategy(std::string_view name);

}  // namespace mms::lb
