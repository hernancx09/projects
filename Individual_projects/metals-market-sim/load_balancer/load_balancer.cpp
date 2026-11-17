#include "load_balancer/load_balancer.hpp"

#include <sys/epoll.h>
#include <sys/socket.h>

#include <cerrno>

#include "common/log.hpp"
#include "common/todo.hpp"

namespace mms::lb {

LoadBalancer::LoadBalancer(EventLoop& loop, BackendList& backends, std::unique_ptr<Strategy> strategy)
    : loop_(loop), backends_(backends), strategy_(std::move(strategy)) {}

void LoadBalancer::listen(uint16_t port) {
    // TODO(P2):
    //   listen_fd_ = net::tcp_listen(port);
    //   net::set_nonblocking(listen_fd_.get());
    //   loop_.add(listen_fd_.get(), EPOLLIN, [this](uint32_t) { on_accept(); });
    //   log::info("listening on :", port, " with ", backends_.size(), " backends");
    not_implemented();
}

void LoadBalancer::on_accept() {
    // TODO(P2):
    //   Loop accept() until EAGAIN (several clients may be waiting):
    //     sockaddr_in addr{}; socklen_t len = sizeof addr;
    //     int cfd = ::accept4(listen_fd_.get(), (sockaddr*)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    //     Backend* b = strategy_->pick(backends_, <client ip as string>);
    //     if (!b) { log::warn("no healthy backends"); ::close(cfd); continue; }
    //     connect to b (blocking net::tcp_connect is OK for phase 2; see the note below)
    //       on failure: that's a PASSIVE health signal. Try another backend, or close the client.
    //     set both nonblocking + nodelay
    //     auto s = std::make_unique<Session>(); fill it in; ++b->active_connections; ++b->total_connections;
    //     register BOTH fds:
    //       loop_.add(client_fd, EPOLLIN, [this, sp = s.get()](uint32_t ev) { on_client_events(*sp, ev); });
    //       loop_.add(backend_fd, EPOLLIN, [this, sp = s.get()](uint32_t ev) { on_backend_events(*sp, ev); });
    //     sessions_[client_fd] = std::move(s);
    //     log::info(client_addr, " -> ", b->name());
    //
    // NOTE: a blocking connect() here freezes the WHOLE LB while it runs. On a
    //   LAN that's ~0.1ms, so it's fine at first. TODO(STRETCH): non-blocking
    //   connect (EINPROGRESS + wait for EPOLLOUT), the same pattern as health.cpp.
    not_implemented();
}

void LoadBalancer::on_client_events(Session& s, uint32_t events) {
    // TODO(P2):
    //   if EPOLLIN:  if (!forward(client, backend, s.to_backend)) -> close_session
    //   if EPOLLOUT: flush s.to_client into the client fd (it had pending bytes)
    //   if EPOLLERR|EPOLLHUP -> close_session
    // BACKPRESSURE: if s.to_backend gets big (backend is slow), STOP reading from
    //   the client (modify it to not include EPOLLIN) until the buffer drains.
    //   Otherwise one fast client + one slow backend = unbounded memory.
    not_implemented();
}

void LoadBalancer::on_backend_events(Session& s, uint32_t events) {
    // TODO(P2): mirror image of on_client_events
    not_implemented();
}

bool LoadBalancer::forward(int from, int to, std::vector<uint8_t>& pending) {
    // TODO(P2):
    //   uint8_t buf[65536]; ssize_t n = ::recv(from, buf, sizeof buf, 0);
    //   n == 0 -> EOF -> return false
    //   n < 0  -> EAGAIN: return true; else return false
    //   If pending is empty, try ::send(to, buf, n, MSG_NOSIGNAL) directly.
    //   Whatever didn't go out gets appended to pending, and you enable EPOLLOUT on `to`.
    not_implemented();
}

void LoadBalancer::close_session(int client_fd) {
    // TODO(P2):
    //   find the session; loop_.remove() BOTH fds; --target->active_connections;
    //   log bytes up/down; erase from sessions_ (the Fds close themselves).
    //   CAREFUL: you're probably inside one of this session's own callbacks. EventLoop
    //   copies the handler's shared_ptr, so erasing is OK, but don't touch `s` afterwards.
    //   (Or use loop_.defer() to erase after the current batch.)
    not_implemented();
}

void LoadBalancer::listen_stats(uint16_t port) {
    // TODO(STRETCH): on accept, write one line per backend:
    //   "gateway1:9100 healthy=1 active=4 total=17\n" and close.
    not_implemented();
}

}  // namespace mms::lb
