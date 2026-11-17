#include "common/connection.hpp"

#include <sys/epoll.h>
#include <sys/socket.h>

#include <cerrno>
#include <cstring>

#include "common/log.hpp"
#include "common/todo.hpp"

namespace mms {

TcpConnection::TcpConnection(EventLoop& loop, net::Fd fd, std::string peer)
    : loop_(loop), fd_(std::move(fd)), peer_(std::move(peer)) {}

TcpConnection::~TcpConnection() = default;

void TcpConnection::start(MessageHandler on_message, CloseHandler on_close) {
    // TODO(P2):
    //   on_message_ = std::move(on_message); on_close_ = std::move(on_close);
    //   net::set_nonblocking(fd()); net::set_nodelay(fd());
    //   Capture a weak_ptr, not `this`, so a callback can't run on a destroyed object:
    //     std::weak_ptr<TcpConnection> weak = shared_from_this();
    //     loop_.add(fd(), EPOLLIN, [weak](uint32_t ev) { if (auto self = weak.lock()) self->on_events(ev); });
    not_implemented();
}

void TcpConnection::on_events(uint32_t events) {
    // TODO(P2):
    //   if (events & (EPOLLERR | EPOLLHUP)) -> close(); return;
    //   if (events & EPOLLIN) on_readable();
    //   if (!closed_ && (events & EPOLLOUT)) on_writable();
    not_implemented();
}

void TcpConnection::on_readable() {
    // TODO(P2):
    //   uint8_t buf[16384];
    //   ssize_t n = ::recv(fd(), buf, sizeof buf, 0);
    //   n > 0  -> in_.append(buf, n); then
    //             Message m; while ((status = in_.next(m)) == Ok) on_message_(*this, m);
    //             if status == Error -> log::warn("protocol error from ", peer_); close();
    //             CAREFUL: on_message_ might close() this connection. Check closed_ in the loop.
    //   n == 0 -> peer closed cleanly (it sent FIN) -> close()
    //   n < 0  -> errno EAGAIN/EWOULDBLOCK: nothing to read, fine, return
    //             errno EINTR: retry; anything else (ECONNRESET...): close()
    //
    // Experiment: log n each time. Send a big burst and watch messages get
    //   split and merged across recv() calls. That's the whole reason FrameBuffer exists.
    not_implemented();
}

void TcpConnection::on_writable() {
    // TODO(P2): flush(); if everything was sent and want_write_: loop_.modify(fd(), EPOLLIN); want_write_ = false;
    not_implemented();
}

void TcpConnection::send(const Message& msg) {
    // TODO(P2):
    //   if (closed_) return;
    //   protocol::encode_frame(msg, out_);
    //   flush();
    //   if bytes remain and !want_write_: loop_.modify(fd(), EPOLLIN | EPOLLOUT); want_write_ = true;
    not_implemented();
}

void TcpConnection::flush() {
    // TODO(P2):
    //   while (out_offset_ < out_.size()) {
    //       ssize_t n = ::send(fd(), out_.data() + out_offset_, out_.size() - out_offset_, MSG_NOSIGNAL);
    //         ^ MSG_NOSIGNAL: without it, writing to a closed peer sends you SIGPIPE,
    //           which KILLS THE PROCESS by default. Classic server bug.
    //       n > 0 -> out_offset_ += n
    //       n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK) -> break (kernel buffer full; wait for EPOLLOUT)
    //       n < 0 otherwise -> close(); return
    //   }
    //   if (out_offset_ == out_.size()) { out_.clear(); out_offset_ = 0; }
    not_implemented();
}

void TcpConnection::close() {
    // TODO(P2):
    //   if (closed_) return; closed_ = true;
    //   loop_.remove(fd());
    //   auto self = shared_from_this();   // stay alive until this function returns, even if on_close_ drops the last owner
    //   if (on_close_) on_close_(*this);
    //   fd_.reset();
    not_implemented();
}

}  // namespace mms
