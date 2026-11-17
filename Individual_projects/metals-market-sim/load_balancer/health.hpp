#pragma once
// =============================================================================
// Active health checks, on a SEPARATE THREAD.
//
// An LB learns a backend is dead in one of two ways:
//   PASSIVE: a real connection to it fails. Free, but a real client suffered.
//   ACTIVE:  probe each backend every N seconds, regardless of traffic. (this file)
//
// Probe levels:
//   1. TCP connect works       -> the process is listening (the kernel accepted it)
//   2. Ping frame gets a Pong  -> the app's event loop is actually responsive
//
// HYSTERESIS: don't flip on one result. Need FAIL_THRESHOLD failures in a row
// to mark DOWN and RISE_THRESHOLD successes in a row to mark UP. Otherwise one
// slow probe makes a backend "flap" in and out of rotation.
//
// WHY A THREAD? A probe does BLOCKING connects with timeouts. In the event
// loop thread that would freeze all proxying. So probes run on their own
// std::thread and communicate only through Backend::healthy (an atomic).
// This is the ONE place in the project with real multithreading. Notice how
// little the threads share, and keep it that way.
// =============================================================================
#include <atomic>
#include <chrono>
#include <thread>

#include "load_balancer/strategies.hpp"

namespace mms::lb {

class HealthChecker {
public:
    static constexpr int kFailThreshold = 3;
    static constexpr int kRiseThreshold = 2;

    HealthChecker(BackendList& backends, std::chrono::milliseconds interval,
                  std::chrono::milliseconds probe_timeout = std::chrono::milliseconds(1000));
    ~HealthChecker();  // stop() + join. NEVER let a std::thread be destroyed while joinable (std::terminate!)

    void start();
    void stop();

private:
    void run();                           // the thread body
    bool probe(const Backend& backend);   // true = alive
    void record(Backend& backend, bool ok);

    BackendList& backends_;  // must NOT be resized while the thread runs
    std::chrono::milliseconds interval_;
    std::chrono::milliseconds probe_timeout_;
    std::atomic<bool> running_{false};
    std::thread thread_;
};

}  // namespace mms::lb
