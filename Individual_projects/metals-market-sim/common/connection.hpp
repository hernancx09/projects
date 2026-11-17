#pragma once
// =============================================================================
// TcpConnection: one non-blocking TCP connection that speaks our framed protocol.
// =============================================================================
// Used by the gateway (to clients and to engines), the engine (to gateways),
// and the client SDK (to the LB). NOT used by the load balancer, which moves
// raw bytes without parsing them.
//
// READ SIDE:  socket readable -> recv() into a buffer -> FrameBuffer::append
//             -> pull out complete Messages -> on_message(msg) for each.
//
// WRITE SIDE: send(msg) encodes into out_ and tries ::send() right away.
//             A non-blocking send() may accept only PART of your bytes
//             (kernel buffer full) or none (EAGAIN). Keep the rest in out_,
//             ask epoll for EPOLLOUT, and send more when the socket is
//             writable. Turn EPOLLOUT off again when out_ is empty, or epoll
//             wakes you constantly (the socket is almost always writable).
//
//             If out_ keeps growing, the peer isn't reading: a SLOW CONSUMER.
//             Real systems cap the buffer and disconnect the slow peer.
//             TODO(STRETCH): do that (kMaxOutBuffer).
//
// NAGLE'S ALGORITHM (TCP_NODELAY): by default the kernel holds back small
// writes for up to ~40ms to batch them into fewer packets. Great for
// bandwidth, terrible for latency. Every trading system disables it.
// TODO(STRETCH): measure order round-trip time with and without it.
//
// LIFETIME: owned by a std::shared_ptr. The owner (gateway/engine) keeps it in
// a map and erases it in on_close. Never touch the connection after that.
// =============================================================================
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "common/event_loop.hpp"
#include "common/messages.hpp"
#include "common/protocol.hpp"
#include "common/socket.hpp"

namespace mms {

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    using MessageHandler = std::function<void(TcpConnection&, const Message&)>;
    using CloseHandler = std::function<void(TcpConnection&)>;

    TcpConnection(EventLoop& loop, net::Fd fd, std::string peer);
    ~TcpConnection();

    // Make non-blocking + TCP_NODELAY, register with the loop for EPOLLIN.
    void start(MessageHandler on_message, CloseHandler on_close);

    void send(const Message& msg);  // queue + try to write now
    void close();                   // remove from loop, close fd, call on_close ONCE

    int fd() const { return fd_.get(); }
    const std::string& peer() const { return peer_; }
    bool closed() const { return closed_; }

    // Free slot for the owner's per-connection data (e.g. the logged-in account).
    std::string tag;

private:
    void on_events(uint32_t events);
    void on_readable();
    void on_writable();
    void flush();  // write as much of out_ as the kernel will take

    EventLoop& loop_;
    net::Fd fd_;
    std::string peer_;
    protocol::FrameBuffer in_;
    std::vector<uint8_t> out_;
    std::size_t out_offset_ = 0;  // bytes of out_ already sent
    bool want_write_ = false;     // is EPOLLOUT currently enabled?
    bool closed_ = false;
    MessageHandler on_message_;
    CloseHandler on_close_;
};

}  // namespace mms
