#include "engine/engine.hpp"

#include <sys/epoll.h>
#include <sys/socket.h>

#include "common/log.hpp"
#include "common/todo.hpp"

namespace mms {

Engine::Engine(EventLoop& loop, Symbol symbol) : loop_(loop), symbol_(symbol), book_(symbol), accounts_(symbol) {}

// =============================================================== networking
void Engine::listen_orders(uint16_t port) {
    // TODO(P1) PHASE 1 GOAL: get this accepting connections and ECHOING every
    //   frame straight back (in on_gateway_message), before any matching
    //   exists. Test it with tools/... or a tiny client. That proves your
    //   protocol + event loop + connection code all work.
    // TODO(P2): tcp_listen, nonblocking, loop_.add(fd, EPOLLIN, [this](uint32_t){ on_order_accept(); })
    not_implemented();
}

void Engine::on_order_accept() {
    // TODO(P2): accept4 loop -> make_shared<TcpConnection> -> start(on_gateway_message, on_gateway_close) -> gateways_[fd]
    not_implemented();
}

void Engine::on_gateway_message(TcpConnection& gw, const Message& msg) {
    // TODO(P2): dispatch:
    //   Ping     -> gw.send(Pong{})
    //   NewOrder -> handle_new_order(gw, ...)
    //   Cancel   -> handle_cancel(gw, ...)
    //   other    -> log::warn and ignore
    not_implemented();
}

void Engine::on_gateway_close(int fd) {
    // TODO(P2): erase from gateways_.
    //   Orders whose owner was this gateway can't get their fills delivered
    //   anymore. The client may already have reconnected through a DIFFERENT
    //   gateway. TODO(STRETCH): route fills by ACCOUNT (gateways tell the
    //   engine "account X is on me now") instead of by order owner.
    //   Or: cancel the orders on disconnect. What do real exchanges do?
    not_implemented();
}

// ================================================================= business
void Engine::handle_new_order(TcpConnection& gw, const NewOrder& msg) {
    // TODO(P3):
    //   1. if (msg.symbol != symbol_) -> Reject(UnknownSymbol)  (a gateway routing bug!)
    //   2. if (auto r = accounts_.check_order(msg.account, msg.side, msg.qty)) -> gw.send(Reject{...}); return;
    //   3. Order order{book_.next_order_id(), msg.account, msg.client_order_id, msg.side, msg.order_type, msg.price, msg.qty, msg.qty};
    //   4. gw.send(OrderAck{msg.account, msg.client_order_id, order.id});
    //   5. auto execs = book_.submit(order);
    //   6. for each Execution e:
    //        accounts_.apply_fill(aggressor account, msg.side, e.price, e.qty)
    //        accounts_.apply_fill(e.resting_account, opposite(msg.side), e.price, e.qty)
    //        aggressor Fill   -> gw.send(...)
    //        resting   Fill   -> gateways_[order_owner_fd_[e.resting_id]]->send(...)  (check it still exists!)
    //        if (e.resting_remaining == 0) erase it from order_owner_fd_ and client_to_order_
    //        md_->publish(Trade{symbol_, e.price, e.qty, msg.side})   <- NO account names in public data
    //        last_trade_ = e.price
    //   7. if the order RESTED (order.qty > 0 && Limit):
    //        order_owner_fd_[order.id] = gw.fd(); client_to_order_[{account, cid}] = order.id
    //        update open_buy_qty / open_sell_qty
    //   8. if Market and order.qty > 0: the rest was dropped. Tell the client (a final
    //        Fill with qty 0? a CancelAck?). Pick one and document it.
    //   9. publish_quote()
    //
    // Log each order + its executions at DEBUG level. With LOG_LEVEL=DEBUG you
    //   can replay exactly what happened.
    not_implemented();
}

void Engine::handle_cancel(TcpConnection& gw, const Cancel& msg) {
    // TODO(P3):
    //   look up client_to_order_[{msg.account, msg.client_order_id}]
    //     missing -> Reject(UnknownOrder) (it probably already filled. Normal race, not a bug)
    //   book_.cancel(id); erase the maps; fix open_*_qty; gw.send(CancelAck{...}); publish_quote();
    //   Keying by (account, cid) means one account can't cancel another's orders.
    not_implemented();
}

void Engine::publish_quote() {
    // TODO(P4): auto q = book_.top_of_book();
    //   only publish if q != last_published_quote_ (operator== is defaulted for you)
    //   md_->publish(q); last_published_quote_ = q;
    not_implemented();
}

void Engine::start_market_data(uint16_t port) {
    // TODO(P4): md_ = std::make_unique<UdpPublisher>(loop_, std::string(to_string(symbol_)), port);
    //           md_->start_heartbeats(std::chrono::milliseconds(1000));
    not_implemented();
}

// ==================================================================== admin
void Engine::listen_admin(uint16_t port) {
    // TODO(P6): tcp_listen + loop_.add(..., on_admin_accept)
    not_implemented();
}

void Engine::on_admin_accept() {
    // TODO(P6): accept; write a short text report and close:
    //   "symbol XAU\nmid 240012.5\nlast 240010\n" + accounts_.snapshot(mid)
    //   A small blocking send() is fine here (the report is tiny).
    //   Try: docker compose exec bots nc engine_xau 9500
    not_implemented();
}

}  // namespace mms
