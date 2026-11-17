#include "common/udp_feed.hpp"

#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <cerrno>

#include "common/log.hpp"
#include "common/protocol.hpp"
#include "common/todo.hpp"

namespace mms {

UdpPublisher::UdpPublisher(EventLoop& loop, std::string name, uint16_t port)
    : loop_(loop), name_(std::move(name)) {
    // TODO(P4):
    //   fd_ = net::udp_socket(port);
    //   net::set_nonblocking(fd_.get());
    //   loop_.add(fd_.get(), EPOLLIN, [this](uint32_t) { on_readable(); });
    //   log::info(name_, " publishing on udp port ", port);
    not_implemented();
}

void UdpPublisher::on_readable() {
    // TODO(P4):
    //   Loop until EAGAIN (several datagrams may be waiting):
    //     uint8_t buf[2048]; sockaddr_in from{}; socklen_t len = sizeof from;
    //     ssize_t n = recvfrom(fd_.get(), buf, sizeof buf, 0, (sockaddr*)&from, &len);
    //     decode it (protocol::decode_datagram, OR a bare frame. Pick one and match client.cpp!)
    //     if it's a Subscribe: subscribers_[{from.sin_addr.s_addr, from.sin_port}] = {from, now}
    //       log the FIRST time you see an address: "new subscriber 172.20.0.7:51234"
    //   NEVER crash on garbage from the network. Log at debug and ignore it.
    not_implemented();
}

void UdpPublisher::publish(const Message& msg) {
    // TODO(P4):
    //   ++seq_;
    //   scratch_.clear();
    //   protocol::encode_datagram(seq_, protocol::now_ns(), msg, scratch_);
    //   for (auto& [key, sub] : subscribers_)
    //       sendto(fd_.get(), scratch_.data(), scratch_.size(), 0, (sockaddr*)&sub.addr, sizeof sub.addr);
    //
    //   sendto() errors:
    //     ECONNREFUSED: an earlier packet to this address got an ICMP "port
    //                   unreachable" back (the subscriber died). Drop it? (STRETCH)
    //     EAGAIN:       our send buffer is full. For market data, dropping is
    //                   usually right: stale prices are worthless. Count drops.
    not_implemented();
}

void UdpPublisher::start_heartbeats(std::chrono::milliseconds interval) {
    // TODO(P4): loop_.add_timer(interval, [this] { publish(Heartbeat{seq_}); });
    //   Q: should heartbeats consume a sequence number? Either works. Decide, and
    //   make the client's gap detector agree.
    not_implemented();
}

}  // namespace mms
