// Token bucket tests. Note how the clock is INJECTED: no sleeping in tests.
#include <gtest/gtest.h>

#include "gateway/token_bucket.hpp"

using namespace mms;
using namespace std::chrono_literals;

TEST(TokenBucket, AllowsBurstUpToCapacity) {
    auto t0 = TokenBucket::Clock::now();
    TokenBucket tb(/*rate*/ 10, /*capacity*/ 3, t0);
    EXPECT_TRUE(tb.allow(t0));
    EXPECT_TRUE(tb.allow(t0));
    EXPECT_TRUE(tb.allow(t0));
    EXPECT_FALSE(tb.allow(t0));  // bucket empty
}

TEST(TokenBucket, RefillsOverTime) {
    auto t0 = TokenBucket::Clock::now();
    TokenBucket tb(10, 1, t0);
    EXPECT_TRUE(tb.allow(t0));
    EXPECT_FALSE(tb.allow(t0 + 50ms));  // only 0.5 tokens back
    EXPECT_TRUE(tb.allow(t0 + 100ms));  // 1 full token back
}

TEST(TokenBucket, NeverExceedsCapacity) {
    auto t0 = TokenBucket::Clock::now();
    TokenBucket tb(10, 2, t0);
    tb.allow(t0 + 10s);  // a long idle time must NOT bank 100 tokens
    EXPECT_LE(tb.tokens(), 2.0);
}
