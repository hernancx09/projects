#pragma once
// =============================================================================
// Token bucket rate limiter.
//
// The bucket holds up to `capacity` tokens and refills at `rate` tokens/sec.
// Each message costs 1 token. Empty bucket = reject.
//   -> allows short BURSTS (up to capacity) but caps the AVERAGE rate at `rate`.
//
// The current time is a PARAMETER (defaulting to now). That makes it
// testable: tests pass in fake times instead of sleeping. This is
// "dependency injection" of the clock. See tests/test_token_bucket.cpp.
//
// steady_clock, NOT system_clock: the system clock can jump (NTP, someone
// changing the time). The steady clock only moves forward.
// =============================================================================
#include <algorithm>
#include <chrono>

#include "common/todo.hpp"

namespace mms {

class TokenBucket {
public:
    using Clock = std::chrono::steady_clock;

    TokenBucket(double rate_per_sec, double capacity, Clock::time_point now = Clock::now())
        : rate_(rate_per_sec), capacity_(capacity), tokens_(capacity), last_(now) {}

    bool allow(Clock::time_point now = Clock::now()) {
        // TODO(P3):
        //   double elapsed = std::chrono::duration<double>(now - last_).count();
        //   tokens_ = std::min(capacity_, tokens_ + elapsed * rate_);
        //   last_ = now;
        //   if (tokens_ >= 1.0) { tokens_ -= 1.0; return true; }
        //   return false;
        not_implemented();
    }

    double tokens() const { return tokens_; }

private:
    double rate_;
    double capacity_;
    double tokens_;
    Clock::time_point last_;
};

}  // namespace mms
