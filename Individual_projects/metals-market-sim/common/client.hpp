#pragma once
// =============================================================================
// ExchangeClient: the ONE way any trader talks to the exchange.
//
// Bots AND your own trader use this class. Nobody gets secret advantages in
// the plumbing, so you win on strategy. (Later you might fork your own faster
// copy. That's fair game too.)
//
// It manages:
//   1. A TCP connection to the load balancer, for orders and replies.
//   2. One UDP socket that receives market data from every engine and
//      (optionally) fundamentals.
//   3. Bookkeeping: client order ids, open orders, per-feed sequence tracking.
//
// It makes NO decisions. It calls your callbacks (std::function members below):
//
//   client.on_fill = [&](const Fill& f) { ... };
//
// Everything runs on the EventLoop thread, so callbacks never run concurrently.
// =============================================================================
#include <netinet/in.h>

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include "common/config.hpp"
#include "common/connection.hpp"
#include "common/event_loop.hpp"
#include "common/messages.hpp"
#include "common/socket.hpp"

namespace mms {

struct OpenOrder {
    ClientOrderId client_order_id = 0;
    Symbol symbol = Symbol::XAU;
    Side side = Side::Buy;
    OrderType type = OrderType::Limit;
    Price price = 0;
    Qty remaining = 0;
    std::optional<OrderId> order_id;  // set once the engine ACKs
    bool cancel_pending = false;
};

// Per-feed statistics: watch these to learn about packet delivery.
struct FeedStats {
    bool have_seq = false;
    uint64_t last_seq = 0;
    uint64_t packets = 0;
    uint64_t gaps = 0;           // number of gap EVENTS
    uint64_t lost_packets = 0;   // total packets missing
    uint64_t duplicates_or_old = 0;
    double avg_latency_us = 0;   // exponential moving average of one-way latency
};

class ExchangeClient {
public:
    ExchangeClient(EventLoop& loop, std::string account, config::HostPort lb,
                   std::map<std::string, config::HostPort> market_data,
                   std::optional<config::HostPort> fundamentals = std::nullopt);

    void connect();    // TCP to the LB (with retries) + send Login
    void subscribe();  // UDP socket + Subscribe to every feed

    // Returns immediately with the id. Does NOT wait for the ack: replies
    // arrive later via callbacks. Waiting on each ack would make you a full
    // network round trip slower per order.
    ClientOrderId send_order(Symbol symbol, Side side, OrderType type, Qty qty, Price price = 0);
    void cancel(ClientOrderId id);

    bool logged_in() const { return logged_in_; }
    const std::string& account() const { return account_; }
    const std::unordered_map<ClientOrderId, OpenOrder>& open_orders() const { return open_orders_; }
    const std::map<std::string, FeedStats>& feed_stats() const { return feed_stats_; }

    // ---- callbacks: assign your own ----
    std::function<void()> on_login;
    std::function<void(const OrderAck&)> on_ack;
    std::function<void(const Fill&)> on_fill;
    std::function<void(const Reject&)> on_reject;
    std::function<void(const CancelAck&)> on_cancel_ack;
    std::function<void(const Quote&)> on_quote;
    std::function<void(const Trade&)> on_trade;
    std::function<void(const FairValue&)> on_fair_value;
    std::function<void(const std::string& feed, uint64_t expected, uint64_t got)> on_gap;
    std::function<void()> on_disconnect;

private:
    void on_tcp_message(const Message& msg);
    void on_udp_readable();
    void handle_datagram(const sockaddr_in& from, const uint8_t* data, std::size_t len);
    void send_subscribes();  // (re)send Subscribe to every feed

    EventLoop& loop_;
    std::string account_;
    config::HostPort lb_;
    std::map<std::string, config::HostPort> market_data_;  // "XAU" -> engine_xau:9300
    std::optional<config::HostPort> fundamentals_;

    std::shared_ptr<TcpConnection> conn_;
    net::Fd udp_fd_;
    // Packets come back from an IP, not a hostname. Resolve once and map
    // (ip, port) -> feed name, so we know which feed each packet belongs to.
    std::map<std::pair<uint32_t, uint16_t>, std::string> feed_by_addr_;
    std::map<std::string, sockaddr_in> feed_addrs_;
    std::map<std::string, FeedStats> feed_stats_;

    bool logged_in_ = false;
    ClientOrderId next_id_ = 1;
    std::unordered_map<ClientOrderId, OpenOrder> open_orders_;
};

}  // namespace mms
