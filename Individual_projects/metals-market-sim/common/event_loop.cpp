#include "common/event_loop.hpp"

#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>

#include "common/log.hpp"
#include "common/todo.hpp"

namespace mms {

EventLoop::EventLoop() {
    // TODO(P2): epoll_fd_.reset(epoll_create1(EPOLL_CLOEXEC)); throw if invalid
    not_implemented();
}

EventLoop::~EventLoop() = default;  // net::Fd closes everything (once you implement ~Fd!)

void EventLoop::add(int fd, uint32_t events, Handler handler) {
    // TODO(P2):
    //   epoll_event ev{}; ev.events = events; ev.data.fd = fd;
    //   epoll_ctl(epoll_fd_.get(), EPOLL_CTL_ADD, fd, &ev)   (throw on error)
    //   handlers_[fd] = std::make_shared<Handler>(std::move(handler));
    //
    // Level-triggered (default) vs EDGE-triggered (EPOLLET):
    //   level: epoll keeps reporting the fd as long as data is waiting. Forgiving.
    //   edge:  reports only when NEW data arrives. You MUST read until EAGAIN or you stall.
    //   Start with level-triggered. TODO(STRETCH): switch to EPOLLET and see what breaks.
    not_implemented();
}

void EventLoop::modify(int fd, uint32_t events) {
    // TODO(P2): epoll_ctl(..., EPOLL_CTL_MOD, ...)
    not_implemented();
}

void EventLoop::remove(int fd) {
    // TODO(P2): epoll_ctl(..., EPOLL_CTL_DEL, fd, nullptr); handlers_.erase(fd);
    //   Ignore errors here (the fd may already be gone).
    not_implemented();
}

void EventLoop::add_timer(std::chrono::milliseconds interval, std::function<void()> cb) {
    // TODO(P2):
    //   int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    //   itimerspec spec{}; spec.it_interval = spec.it_value = {seconds, nanoseconds} from interval
    //   timerfd_settime(tfd, 0, &spec, nullptr);
    //   add(tfd, EPOLLIN, [tfd, cb](uint32_t) {
    //       uint64_t expirations; ::read(tfd, &expirations, sizeof expirations);  // MUST read, or it stays "readable" forever
    //       cb();
    //   });
    //   timer_fds_.emplace_back(tfd);   // keep ownership so it gets closed
    //
    // If `expirations` > 1, the loop was too slow and timer ticks got merged.
    // Worth a log::warn: it means something is blocking the loop.
    not_implemented();
}

void EventLoop::defer(std::function<void()> cb) { deferred_.push_back(std::move(cb)); }

void EventLoop::run() {
    // TODO(P2):
    //   running_ = true;
    //   epoll_event events[64];
    //   while (running_) {
    //       int n = epoll_wait(epoll_fd_.get(), events, 64, -1);   // -1 = wait forever
    //       if (n < 0) { if (errno == EINTR) continue; throw ...; }   // EINTR: interrupted by a signal, just retry
    //       for (int i = 0; i < n; ++i) {
    //           auto it = handlers_.find(events[i].data.fd);
    //           if (it == handlers_.end()) continue;   // removed by an earlier handler in this same batch
    //           auto handler = it->second;             // COPY the shared_ptr: keeps it alive if it removes itself
    //           (*handler)(events[i].events);
    //       }
    //       run the deferred_ callbacks (swap into a local vector first; a callback may defer() more)
    //   }
    //
    // TODO(STRETCH): exceptions. A throw from one handler currently kills the
    //   whole process. Catch per-handler, log, and close just that connection?
    not_implemented();
}

void EventLoop::stop() { running_ = false; }

}  // namespace mms
