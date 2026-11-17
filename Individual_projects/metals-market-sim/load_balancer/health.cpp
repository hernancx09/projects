#include "load_balancer/health.hpp"

#include <poll.h>
#include <sys/socket.h>

#include <cerrno>

#include "common/log.hpp"
#include "common/socket.hpp"
#include "common/todo.hpp"

namespace mms::lb {

HealthChecker::HealthChecker(BackendList& backends, std::chrono::milliseconds interval,
                             std::chrono::milliseconds probe_timeout)
    : backends_(backends), interval_(interval), probe_timeout_(probe_timeout) {}

HealthChecker::~HealthChecker() { stop(); }

void HealthChecker::start() {
    // TODO(P2): running_ = true; thread_ = std::thread([this] { run(); });
    not_implemented();
}

void HealthChecker::stop() {
    // TODO(P2): running_ = false; if (thread_.joinable()) thread_.join();
    //   (Destructors must not throw, so this doesn't call not_implemented().)
    //   Note: join() waits up to one interval for run() to wake up. A
    //   std::condition_variable would let stop() wake it immediately. (STRETCH)
}

void HealthChecker::run() {
    // TODO(P2):
    //   while (running_) {
    //       for (auto& b : backends_) record(*b, probe(*b));
    //       std::this_thread::sleep_for(interval_);    // fine HERE: this is not the event loop thread
    //   }
    // TODO(STRETCH): probe all backends IN PARALLEL. With 3 sequential
    //   probes and a 1s timeout, one dead backend delays checking the others.
    not_implemented();
}

bool HealthChecker::probe(const Backend& backend) {
    // TODO(P2) level 1, a TCP connect WITH A TIMEOUT. A plain blocking connect()
    //   to a dead host can hang for over a minute. The standard trick:
    //     1. socket(), set_nonblocking
    //     2. connect() -> returns -1 with errno == EINPROGRESS (the handshake has started)
    //     3. poll({fd, POLLOUT}, timeout_ms) -> waits until the handshake finishes or times out
    //     4. getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len) -> err == 0 means connected
    //   Learning this pattern pays off: it's how every async client connects.
    //
    // TODO(P2) level 2: after connecting, send a Ping frame and expect a Pong
    //   (poll for POLLIN with a timeout, then recv). The gateway must answer Ping.
    not_implemented();
}

void HealthChecker::record(Backend& backend, bool ok) {
    // TODO(P2):
    //   ok  -> ++consecutive_successes, consecutive_failures = 0
    //          if !healthy and successes >= kRiseThreshold -> healthy = true, log::info(name, " UP")
    //   !ok -> ++consecutive_failures, consecutive_successes = 0
    //          if healthy and failures >= kFailThreshold -> healthy = false, log::warn(name, " DOWN")
    //   LOG EVERY FLIP. When you `docker compose stop gateway2`, these lines are what you look for.
    //
    // Q: what happens to clients ALREADY connected through a backend that goes down?
    //   The LB doesn't kill them. Their connection breaks, and the CLIENT must
    //   reconnect (and lands on a healthy backend). Watch it happen.
    not_implemented();
}

}  // namespace mms::lb
