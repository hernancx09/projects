#pragma once
// =============================================================================
// Gateway: the front door of the exchange. 3 identical replicas sit behind the LB.
//
// Jobs:
//   1. SESSION      the first message must be Login (or Ping from a health check).
//                   Remember which account owns each connection.
//   2. VALIDATE     reject garbage before it reaches the engine.
//   3. RATE LIMIT   a buggy bot must not flood the engine (TokenBucket).
//   4. ROUTE        msg.symbol -> that symbol's engine. (Sharding: the gateway
//                   is the "router" that knows the shard map.)
//   5. RETURN PATH  engine replies (Ack/Fill/Reject) -> the right client.
//
// WHY STATELESS? Nothing important lives here. Orders and positions live in
// the engines. If a gateway dies, clients reconnect through the LB to
// another one and lose nothing. That's what makes this tier easy to scale:
// just add gateway4.
//
// CONNECTIONS:
//   client side: many TcpConnections IN (via the LB)
//   engine side: ONE TcpConnection OUT per engine, shared by every client on
//                this gateway ("multiplexing")
// Because the engine connection is shared, every engine reply carries the
// ACCOUNT. That's how we find the right client again.
// =============================================================================
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include "common/config.hpp"
#include "common/connection.hpp"
#include "common/event_loop.hpp"
#include "common/messages.hpp"
#include "gateway/token_bucket.hpp"

namespace mms {

class Gateway {
public:
    Gateway(EventLoop& loop, std::string gateway_id, std::map<std::string, config::HostPort> engines,
            double rate_limit_per_sec);

    void connect_engines();       // blocking + retries, at startup
    void listen(uint16_t port);

    // Pure function: easy to unit test. nullopt = message is OK.
    static std::optional<RejectReason> validate(const NewOrder& order);

private:
    struct ClientSession {
        std::shared_ptr<TcpConnection> conn;
        std::optional<std::string> account;  // set after Login
        TokenBucket bucket;
    };

    void on_accept();
    void on_client_message(ClientSession& session, const Message& msg);
    void on_client_close(int fd);
    void on_engine_message(Symbol symbol, const Message& msg);
    void reject(ClientSession& session, ClientOrderId id, RejectReason reason);

    EventLoop& loop_;
    std::string id_;
    std::map<std::string, config::HostPort> engine_addrs_;
    double rate_limit_;
    net::Fd listen_fd_;

    std::unordered_map<Symbol, std::shared_ptr<TcpConnection>> engines_;
    std::unordered_map<int, ClientSession> clients_;           // client fd -> session
    std::unordered_map<std::string, int> account_to_fd_;       // routes engine replies back
};

}  // namespace mms
