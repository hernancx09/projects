#include "load_balancer/strategies.hpp"

#include <stdexcept>

#include "common/todo.hpp"

namespace mms::lb {

Backend* RoundRobin::pick(const BackendList& backends, std::string_view client_ip) {
    // TODO(P2):
    //   Try at most backends.size() times:
    //     Backend* b = backends[next_ % backends.size()].get(); ++next_;
    //     if (b->healthy.load()) return b;
    //   return nullptr;
    // Edge case: backends is empty -> % 0 is a crash (division by zero). Guard it.
    not_implemented();
}

Backend* LeastConnections::pick(const BackendList& backends, std::string_view client_ip) {
    // TODO(P2): scan healthy backends, keep the one with the smallest active_connections.
    //   Ties: the first one wins. (Is that a problem? With 0 connections everywhere,
    //   the first backend gets the first connection every time after a restart...)
    not_implemented();
}

Backend* IpHash::pick(const BackendList& backends, std::string_view client_ip) {
    // TODO(P2):
    //   Hash the IP with something STABLE, like FNV-1a (look it up; it's 5 lines).
    //   std::hash<std::string_view> works too, but its values aren't guaranteed
    //   to be the same across compilers or versions.
    //   idx = hash % number_of_HEALTHY_backends -> the idx-th healthy one.
    //
    // PROBLEM to discover: when 1 of 3 backends dies, `% 3` becomes `% 2` and
    //   almost EVERY client maps somewhere new. CONSISTENT HASHING fixes this:
    //   only ~1/N of clients move. (DynamoDB, Cassandra, and CDNs all use it.)
    not_implemented();
}

std::unique_ptr<Strategy> make_strategy(std::string_view name) {
    // TODO(P2): if (name == "round_robin") return std::make_unique<RoundRobin>(); ...
    //   throw std::invalid_argument for unknown names (include the name in the message!)
    not_implemented();
}

}  // namespace mms::lb
