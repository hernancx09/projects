#include "gateway/gateway.hpp"

#include <sys/epoll.h>
#include <sys/socket.h>

#include <cerrno>

#include "common/log.hpp"
#include "common/todo.hpp"

namespace mms {

Gateway::Gateway(EventLoop& loop, std::string gateway_id, std::map<std::string, config::HostPort> engines,
                 double rate_limit_per_sec)
    : loop_(loop), id_(std::move(gateway_id)), engine_addrs_(std::move(engines)), rate_limit_(rate_limit_per_sec) {}

// ============================================================== engine side
void Gateway::connect_engines() {
    // TODO(P2):
    //   for (auto& [sym_name, hp] : engine_addrs_):
    //     Symbol sym = *parse_symbol(sym_name);
    //     auto fd = net::tcp_connect_retry(hp.host, hp.port, 30, std::chrono::seconds(1));
    //       ^ engines may start after us. docker compose `depends_on` only waits for
    //         the container to START, not for the port to be listening.
    //     auto conn = std::make_shared<TcpConnection>(loop_, std::move(fd), hp.host);
    //     conn->start([this, sym](TcpConnection&, const Message& m) { on_engine_message(sym, m); },
    //                 [this, sym](TcpConnection&) { log::error("lost engine ", sym); engines_.erase(sym); });
    //     engines_[sym] = conn;
    //
    // TODO(P3): when an engine connection drops, reconnect in the background
    //   (a timer that retries), and until then reject orders for that symbol
    //   with RejectReason::EngineUnavailable.
    not_implemented();
}

void Gateway::on_engine_message(Symbol symbol, const Message& msg) {
    // TODO(P2):
    //   Every reply type (OrderAck, Fill, Reject, CancelAck) has an `account` field.
    //   std::visit or get_if to pull it out, then:
    //     auto it = account_to_fd_.find(account);
    //     if (it == end) -> that client already disconnected. Drop it (log::debug).
    //     else clients_.at(it->second).conn->send(msg);
    //   Pong from the engine -> ignore.
    //
    // HEAD-OF-LINE BLOCKING: because send() only BUFFERS (TcpConnection never
    //   blocks), one slow client can't stall delivery to the others. Their
    //   own out buffer just grows. In a blocking design, it would stall everyone.
    not_implemented();
}

// ============================================================== client side
void Gateway::listen(uint16_t port) {
    // TODO(P2): same pattern as LoadBalancer::listen: tcp_listen, nonblocking, loop_.add(..., on_accept)
    not_implemented();
}

void Gateway::on_accept() {
    // TODO(P2): accept4() until EAGAIN; for each client fd:
    //   auto conn = std::make_shared<TcpConnection>(loop_, net::Fd(cfd), peer);
    //   clients_.emplace(cfd, ClientSession{conn, std::nullopt, TokenBucket(rate_limit_, rate_limit_ * 2)});
    //   conn->start([this, cfd](TcpConnection&, const Message& m) { on_client_message(clients_.at(cfd), m); },
    //               [this, cfd](TcpConnection&) { on_client_close(cfd); });
    //   Note `peer` will be the LB's address, not the bot's. See load_balancer.hpp.
    not_implemented();
}

void Gateway::on_client_message(ClientSession& s, const Message& msg) {
    // TODO(P2):
    //   Ping -> send Pong, then close (that was the LB's health check)
    //   Login:
    //     if already logged in -> reject
    //     if account_to_fd_ already has this account (logged in on another connection)
    //       -> decide: reject the new one (DuplicateLogin) or kick the old one
    //     else s.account = login.account; account_to_fd_[...] = fd; send LoginAck
    //     (TODO(STRETCH): real auth with a shared secret + HMAC)
    //   anything else before Login -> Reject(NotLoggedIn)
    //
    //   NewOrder / Cancel (after login):
    //     TODO(P3): if (!s.bucket.allow()) -> Reject(RateLimited)
    //     TODO(P3): if (auto r = validate(order)) -> Reject(*r)
    //     OVERWRITE order.account = *s.account. NEVER trust the client to say who it
    //       is on every message, or a bot could trade on someone else's account.
    //     forward to engines_[order.symbol] (missing -> Reject(EngineUnavailable))
    not_implemented();
}

void Gateway::on_client_close(int fd) {
    // TODO(P2): erase from account_to_fd_ (only if it still points at THIS fd!) and from clients_.
    //   Use loop_.defer() for the erase if you're inside the session's own callback.
    not_implemented();
}

void Gateway::reject(ClientSession& s, ClientOrderId id, RejectReason reason) {
    // TODO(P2): s.conn->send(Reject{s.account.value_or(""), id, reason});
    not_implemented();
}

std::optional<RejectReason> Gateway::validate(const NewOrder& o) {
    // TODO(P3): think like an attacker. What input could crash or confuse the engine?
    //   qty <= 0                     -> InvalidQty
    //   qty > some max (1000?)       -> InvalidQty
    //   Limit with price <= 0        -> InvalidPrice
    //   symbol / side / type not one of the enum values -> BadMessage
    //     (decode_body should already catch this. Defense in depth.)
    //   return std::nullopt if OK
    not_implemented();
}

}  // namespace mms
