#pragma once
// =============================================================================
// EventLoop: one thread handling many sockets at once, with epoll.
// =============================================================================
//
// THE PROBLEM: recv() on a blocking socket WAITS until data arrives. With 30
// clients you can't block on one of them while the other 29 are talking.
//
// Option A: one thread per connection. Simple to write. But threads cost
//           memory, and shared state (the order book!) then needs locks.
// Option B: NON-BLOCKING sockets + an EVENT LOOP (this file). One thread asks
//           the kernel "which of these fds are ready?", handles the ready
//           ones, and asks again. This is how nginx, Redis, Node.js, and
//           most trading gateways work.
//
// epoll (Linux):
//   epoll_create1(0)                 -> an epoll fd (a "watch list")
//   epoll_ctl(ep, ADD, fd, events)   -> watch fd for EPOLLIN (readable) / EPOLLOUT (writable)
//   epoll_wait(ep, out, max, timeout)-> BLOCKS until some fds are ready, fills `out`
//
// Readable means: data waiting, OR a new connection to accept (listening
// socket), OR the peer closed (recv returns 0).
// Writable means: there's room in the kernel send buffer again.
//
// Single-threaded = NO LOCKS. Callbacks run one at a time, so the engine's
// order book can never be touched by two things at once. (Real matching
// engines are single-threaded per book for this reason. Determinism matters.)
//
// THE ONE RULE: never block inside a callback (no sleep, no blocking
// connect, no blocking DNS). While one callback blocks, EVERY socket waits.
//
// Usage:
//   EventLoop loop;
//   loop.add(listen_fd, EPOLLIN, [&](uint32_t ev) { /* accept() */ });
//   loop.add_timer(std::chrono::milliseconds(500), [&] { /* every 500ms */ });
//   loop.run();   // forever
// =============================================================================
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "common/socket.hpp"

namespace mms {

class EventLoop {
public:
    using Handler = std::function<void(uint32_t events)>;  // events = bitmask of EPOLLIN/EPOLLOUT/EPOLLERR/EPOLLHUP

    EventLoop();
    ~EventLoop();
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    void add(int fd, uint32_t events, Handler handler);
    void modify(int fd, uint32_t events);  // e.g. add EPOLLOUT when you have unsent data
    void remove(int fd);                   // call BEFORE closing the fd

    // Call `cb` every `interval`, starting one interval from now.
    // Implemented with timerfd: the kernel turns a timer into an fd that
    // becomes readable when it fires, so timers are just more fds in epoll.
    void add_timer(std::chrono::milliseconds interval, std::function<void()> cb);

    // Run `cb` once, after the current batch of events (safe place to delete things).
    void defer(std::function<void()> cb);

    void run();   // loop until stop()
    void stop();

private:
    net::Fd epoll_fd_;
    // shared_ptr so a handler can safely remove() ITSELF while running
    // (copy the shared_ptr before calling it, and the function stays alive).
    std::unordered_map<int, std::shared_ptr<Handler>> handlers_;
    std::vector<net::Fd> timer_fds_;
    std::vector<std::function<void()>> deferred_;
    bool running_ = false;
};

}  // namespace mms
