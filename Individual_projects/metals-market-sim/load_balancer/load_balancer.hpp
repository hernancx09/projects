#pragma once
// =============================================================================
// The load balancer: a Layer-4 TCP proxy.
//
//    client ===TCP#1===> [ LB ] ===TCP#2===> gateway_N
//
// For each client:
//   1. accept() TCP#1
//   2. strategy->pick() a backend
//   3. open TCP#2 to it
//   4. copy bytes both ways until either side closes, then close the other
//
// The LB NEVER parses messages. It moves bytes. A frame might arrive in one
// recv() here and leave in two send()s, so the endpoints' FrameBuffers do
// the reassembly.
//
// The gateway sees the LB's IP as the source, NOT the client's, so the
// original address is lost. Real LBs fix this with the PROXY protocol
// header (L4) or X-Forwarded-For (L7). TODO(STRETCH): implement a simple
// PROXY-protocol-style first line so gateways can log real client IPs.
//
// HALF-CLOSE: TCP lets one side say "I'm done sending" (FIN) while still
// receiving. A correct proxy forwards that with shutdown(fd, SHUT_WR) instead
// of closing everything. Our clients don't need it. Closing both is fine to
// start. TODO(STRETCH): do it properly.
//
// TODO(STRETCH): zero-copy forwarding with splice(2) through a pipe, so bytes
//   never enter user space. Measure throughput before/after.
// =============================================================================
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "common/event_loop.hpp"
#include "common/socket.hpp"
#include "load_balancer/strategies.hpp"

namespace mms::lb {

class LoadBalancer {
public:
    LoadBalancer(EventLoop& loop, BackendList& backends, std::unique_ptr<Strategy> strategy);
    void listen(uint16_t port);

    // Optional stats endpoint: connect, get a text dump of backend status, disconnect.
    //   docker compose exec load_balancer nc localhost 9001
    void listen_stats(uint16_t port);

private:
    // One proxied client <-> backend pair.
    struct Session {
        net::Fd client;
        net::Fd backend;
        Backend* target = nullptr;
        std::vector<uint8_t> to_client;   // bytes read from backend, not yet written to client
        std::vector<uint8_t> to_backend;  // bytes read from client, not yet written to backend
        uint64_t bytes_up = 0, bytes_down = 0;  // stats: log them on close
    };

    void on_accept();
    void on_client_events(Session& s, uint32_t events);
    void on_backend_events(Session& s, uint32_t events);
    // Read from `from` and write to `to`; buffer what `to` can't take yet.
    // Returns false if the session should close.
    bool forward(int from, int to, std::vector<uint8_t>& pending);
    void close_session(int client_fd);

    EventLoop& loop_;
    BackendList& backends_;
    std::unique_ptr<Strategy> strategy_;
    net::Fd listen_fd_;
    net::Fd stats_fd_;
    std::unordered_map<int, std::unique_ptr<Session>> sessions_;  // key = client fd
};

}  // namespace mms::lb
