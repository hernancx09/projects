#pragma once
// =============================================================================
// UdpPublisher: one sender, many receivers, over UDP.
//
// Used by: engine (Quote, Trade, Heartbeat) and fundamentals (FairValue).
//
// HOW SUBSCRIBING WORKS
// ---------------------
// UDP has no "connection". A client subscribes by sending one Subscribe
// datagram TO the publisher's port. recvfrom() tells the publisher the
// (ip, port) it came FROM. The publisher remembers that address and sends
// every update there.
//
//     client : sendto(Subscribe) -> engine_xau:9300
//     engine : recvfrom() -> from 172.20.0.7:51234 -> add to subscribers
//     engine : sendto(Quote) -> 172.20.0.7:51234   (over and over)
//
// (NAT hole punching and multiplayer games use the same trick.)
//
// Real exchanges use IP MULTICAST: send once, and the switches copy the
// packet to everyone in the group. Docker's default bridge network doesn't
// handle multicast well, so we loop over subscribers with unicast sendto().
// At 10,000 subscribers that loop is the bottleneck. That's why multicast exists.
// TODO(STRETCH): real multicast on a macvlan network (IP_ADD_MEMBERSHIP).
// =============================================================================
#include <netinet/in.h>

#include <chrono>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "common/event_loop.hpp"
#include "common/messages.hpp"
#include "common/socket.hpp"

namespace mms {

class UdpPublisher {
public:
    UdpPublisher(EventLoop& loop, std::string name, uint16_t port);

    // Send one message to every subscriber, with the next sequence number.
    // ONE seq per message (not per subscriber), so everyone sees the same numbers.
    void publish(const Message& msg);

    // Send a Heartbeat{last_seq} every `interval` so clients can tell
    // "quiet market" from "I'm missing packets".
    void start_heartbeats(std::chrono::milliseconds interval);

    std::size_t subscriber_count() const { return subscribers_.size(); }
    uint64_t last_seq() const { return seq_; }

private:
    void on_readable();  // a client sent us a datagram (probably Subscribe)

    struct Subscriber {
        sockaddr_in addr{};
        std::chrono::steady_clock::time_point last_seen;
    };

    EventLoop& loop_;
    std::string name_;
    net::Fd fd_;
    // key = (ip, port) as integers, so std::map can order it
    std::map<std::pair<uint32_t, uint16_t>, Subscriber> subscribers_;
    uint64_t seq_ = 0;             // seq of the LAST packet sent
    std::vector<uint8_t> scratch_; // reused encode buffer: no allocation per publish

    // TODO(STRETCH): expire subscribers that haven't re-sent Subscribe in N seconds
    //   (clients re-subscribe periodically, i.e. a "lease"). Otherwise dead bots get packets forever.
    // TODO(STRETCH): keep the last ~1000 datagrams in a ring buffer and serve
    //   retransmit requests over TCP ("gap fill"). That's how real feeds recover lost packets.
};

}  // namespace mms
