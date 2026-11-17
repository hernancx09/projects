#pragma once
// =============================================================================
// Engine: the server that owns the order book for ONE symbol (env SYMBOL).
//
//   gateways --TCP:9200--> [ Engine ] --UDP:9300--> market data subscribers
//                              |
//                              +--TCP:9500--> admin text dump (leaderboard)
//
// Single-threaded on an EventLoop, so the book has exactly ONE writer: no
// locks, no races, and DETERMINISTIC results (the same input sequence always
// gives the same trades). That's why real matching engines look like this.
//
// Note that TcpConnection::send() only BUFFERS bytes. It never blocks. So
// sending replies right after matching is fine: matching never waits on the network.
// =============================================================================
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

#include "common/connection.hpp"
#include "common/event_loop.hpp"
#include "common/messages.hpp"
#include "common/udp_feed.hpp"
#include "engine/accounts.hpp"
#include "engine/order_book.hpp"

namespace mms {

class Engine {
public:
    Engine(EventLoop& loop, Symbol symbol);

    void listen_orders(uint16_t port);
    void start_market_data(uint16_t port);
    void listen_admin(uint16_t port);

private:
    void on_order_accept();
    void on_gateway_message(TcpConnection& gw, const Message& msg);
    void on_gateway_close(int fd);

    void handle_new_order(TcpConnection& gw, const NewOrder& msg);
    void handle_cancel(TcpConnection& gw, const Cancel& msg);
    void publish_quote();      // after anything that changes the top of book
    void on_admin_accept();

    EventLoop& loop_;
    Symbol symbol_;
    OrderBook book_;
    Accounts accounts_;
    std::unique_ptr<UdpPublisher> md_;
    std::optional<Price> last_trade_;
    Quote last_published_quote_{};

    net::Fd order_listen_fd_;
    net::Fd admin_listen_fd_;
    std::unordered_map<int, std::shared_ptr<TcpConnection>> gateways_;  // fd -> connection

    // A resting order can fill MUCH later, when someone else trades against it.
    // Then we need to know where to send that Fill: which gateway owns it?
    std::unordered_map<OrderId, int> order_owner_fd_;
    // Clients cancel by THEIR client_order_id. The book uses OUR OrderId.
    std::map<std::pair<std::string, ClientOrderId>, OrderId> client_to_order_;
};

}  // namespace mms
