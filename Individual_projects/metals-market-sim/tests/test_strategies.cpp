// Load-balancing strategy tests (pure logic, no sockets).
#include <gtest/gtest.h>

#include "load_balancer/strategies.hpp"

using namespace mms::lb;

namespace {

BackendList make_backends(int n) {
    BackendList list;
    for (int i = 0; i < n; ++i) {
        auto b = std::make_unique<Backend>();
        b->host = "gw" + std::to_string(i + 1);
        b->port = 9100;
        list.push_back(std::move(b));
    }
    return list;
}

}  // namespace

TEST(Strategies, RoundRobinCycles) {
    auto backends = make_backends(3);
    RoundRobin rr;
    EXPECT_EQ(rr.pick(backends, "1.1.1.1"), backends[0].get());
    EXPECT_EQ(rr.pick(backends, "1.1.1.1"), backends[1].get());
    EXPECT_EQ(rr.pick(backends, "1.1.1.1"), backends[2].get());
    EXPECT_EQ(rr.pick(backends, "1.1.1.1"), backends[0].get());
}

TEST(Strategies, RoundRobinSkipsUnhealthy) {
    auto backends = make_backends(3);
    backends[1]->healthy = false;
    RoundRobin rr;
    for (int i = 0; i < 10; ++i) EXPECT_NE(rr.pick(backends, "x"), backends[1].get());
}

TEST(Strategies, NoHealthyBackendsReturnsNull) {
    auto backends = make_backends(2);
    for (auto& b : backends) b->healthy = false;
    RoundRobin rr;
    LeastConnections lc;
    EXPECT_EQ(rr.pick(backends, "x"), nullptr);
    EXPECT_EQ(lc.pick(backends, "x"), nullptr);
}

TEST(Strategies, EmptyListReturnsNull) {
    BackendList none;
    RoundRobin rr;
    EXPECT_EQ(rr.pick(none, "x"), nullptr);  // and doesn't divide by zero!
}

TEST(Strategies, LeastConnectionsPicksIdlest) {
    auto backends = make_backends(3);
    backends[0]->active_connections = 5;
    backends[1]->active_connections = 1;
    backends[2]->active_connections = 3;
    LeastConnections lc;
    EXPECT_EQ(lc.pick(backends, "x"), backends[1].get());
}

TEST(Strategies, IpHashIsSticky) {
    auto backends = make_backends(3);
    IpHash h;
    Backend* first = h.pick(backends, "172.20.0.7");
    for (int i = 0; i < 10; ++i) EXPECT_EQ(h.pick(backends, "172.20.0.7"), first);
}

TEST(Strategies, FactoryRejectsUnknownName) {
    EXPECT_THROW(make_strategy("random_guess"), std::invalid_argument);
    EXPECT_NE(make_strategy("round_robin"), nullptr);
}
