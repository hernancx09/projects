#include "common/client.hpp"

#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <cerrno>
#include <chrono>

#include "common/log.hpp"
#include "common/protocol.hpp"
#include "common/todo.hpp"

namespace mms {

ExchangeClient::ExchangeClient(EventLoop& loop, std::string account, config::HostPort lb,
                               std::map<std::string, config::HostPort> market_data,
                               std::optional<config::HostPort> fundamentals)
    : loop_(loop),
      account_(std::move(account)),
      lb_(std::move(lb)),
      market_data_(std::move(market_data)),
      fundamentals_(std::move(fundamentals)) {}

// ================================================================ TCP side
void ExchangeClient::connect() {
    // TODO(P2):
    //   auto fd = net::tcp_connect_retry(lb_.host, lb_.port, 30, std::chrono::seconds(1));
    //     (blocking is OK here: it happens at startup, before loop.run())
    //   conn_ = std::make_shared<TcpConnection>(loop_, std::move(fd), lb_.host);
    //   conn_->start(
    //       [this](TcpConnection&, const Message& m) { on_tcp_message(m); },
    //       [this](TcpConnection&) { logged_in_ = false; log::error(account_, " disconnected"); if (on_disconnect) on_disconnect(); });
    //   conn_->send(Login{account_});
    //
    // TODO(STRETCH): reconnect automatically. And think: what happens to your
    //   open orders on the exchange while you're gone? (Real exchanges offer
    //   "cancel on disconnect".)
    not_implemented();
}

ClientOrderId ExchangeClient::send_order(Symbol symbol, Side side, OrderType type, Qty qty, Price price) {
    // TODO(P2):
    //   ClientOrderId id = next_id_++;
    //   open_orders_[id] = OpenOrder{id, symbol, side, type, price, qty};
    //   conn_->send(NewOrder{account_, id, symbol, side, type, qty, price});
    //   return id;
    // Timestamp it here if you want to measure order -> ack round-trip time later.
    not_implemented();
}

void ExchangeClient::cancel(ClientOrderId id) {
    // TODO(P3): look up the order (need its symbol for routing), mark cancel_pending,
    //   send Cancel{account_, id, symbol}. Skip if unknown or already cancel_pending.
    not_implemented();
}

void ExchangeClient::on_tcp_message(const Message& msg) {
    // TODO(P2): dispatch with std::get_if or std::visit:
    //   LoginAck  -> logged_in_ = true; on_login()
    //   OrderAck  -> open_orders_[cid].order_id = ack.order_id; on_ack()
    //   Fill      -> update remaining; erase when remaining_qty == 0; on_fill()
    //   Reject    -> erase from open_orders_; log::warn; on_reject()
    //   CancelAck -> erase; on_cancel_ack()
    //
    // RACE TO THINK ABOUT: you send Cancel, and at the same moment someone
    //   trades against the order. You may get a Fill and THEN a Reject(UnknownOrder)
    //   for the cancel. Your code must handle that without crashing.
    not_implemented();
}

// ================================================================ UDP side
void ExchangeClient::subscribe() {
    // TODO(P4):
    //   udp_fd_ = net::udp_socket(0);   // any free local port
    //   net::set_nonblocking(udp_fd_.get());
    //   for each (name, hp) in market_data_ (and fundamentals_ as "FUND"):
    //       auto addr = net::resolve(hp.host, hp.port)   (throw/log if it fails)
    //       feed_addrs_[name] = *addr; feed_by_addr_[{addr->sin_addr.s_addr, addr->sin_port}] = name;
    //   loop_.add(udp_fd_.get(), EPOLLIN, [this](uint32_t) { on_udp_readable(); });
    //   send_subscribes();
    //   Subscribe datagrams can be LOST too! Re-send them on a timer:
    //   loop_.add_timer(std::chrono::seconds(5), [this] { send_subscribes(); });
    not_implemented();
}

void ExchangeClient::send_subscribes() {
    // TODO(P4): encode a Subscribe (match what UdpPublisher::on_readable expects) and sendto() each feed_addrs_ entry
    not_implemented();
}

void ExchangeClient::on_udp_readable() {
    // TODO(P4): loop recvfrom() until EAGAIN, calling handle_datagram() for each packet
    not_implemented();
}

void ExchangeClient::handle_datagram(const sockaddr_in& from, const uint8_t* data, std::size_t len) {
    // TODO(P4):
    //   feed = feed_by_addr_[{from.sin_addr.s_addr, from.sin_port}]   (unknown sender -> ignore)
    //   auto dg = protocol::decode_datagram({data, len}); if (!dg) { log::debug; return; }
    //   auto& st = feed_stats_[feed]; ++st.packets;
    //
    //   GAP DETECTION:
    //     if (st.have_seq) {
    //         if (dg->seq <= st.last_seq)       -> duplicate or late (reordered) packet: ++duplicates_or_old; DROP IT
    //         else if (dg->seq > st.last_seq+1) -> GAP: lost (seq - last - 1) packets; ++gaps; on_gap(feed, last+1, seq)
    //     }
    //     st.last_seq = dg->seq; st.have_seq = true;
    //   Heartbeat: if hb.last_seq > st.last_seq you missed the tail end. Also a gap!
    //
    //   LATENCY: (protocol::now_ns() - dg->send_ns) / 1000.0 = one-way microseconds.
    //     EMA: avg = 0.99 * avg + 0.01 * sample
    //
    //   then dispatch: Quote -> on_quote, Trade -> on_trade, FairValue -> on_fair_value
    //
    // With `tools/netem.sh loss 5%` running, watch st.gaps climb.
    // With `netem reorder`, watch duplicates_or_old climb.
    not_implemented();
}

}  // namespace mms
