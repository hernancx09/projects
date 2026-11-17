// Entry point for the fundamentals container: publishes FairValue over UDP.
//   Env: PORT, XAU_START, XAG_START, XAU_VOL, XAG_VOL, CORRELATION, TICK_MS, SIM_SPEED, SEED
#include <exception>

#include "common/config.hpp"
#include "common/event_loop.hpp"
#include "common/log.hpp"
#include "common/udp_feed.hpp"
#include "fundamentals/gbm.hpp"

namespace {
constexpr double kSecondsPerYear = 365.0 * 24 * 3600;
}

int main() {
    mms::log::init("fundamentals");
    try {
        // TODO(P5):
        //   read env vars (see the header comment)
        //   mms::EventLoop loop;
        //   mms::UdpPublisher pub(loop, "fundamentals", port);
        //   pub.start_heartbeats(std::chrono::seconds(1));
        //   mms::CorrelatedGbm model(xau_start, xag_start, xau_vol, xag_vol, rho, 0.0, seed);
        //
        //   SIM_SPEED: 1 real second = SIM_SPEED simulated seconds. Real gold
        //   moves ~1% a DAY, which is boring to watch. Speeding up time makes the
        //   market visibly move.
        //   double dt = (tick_ms / 1000.0) * sim_speed / kSecondsPerYear;
        //
        //   int n = 0;
        //   loop.add_timer(std::chrono::milliseconds(tick_ms), [&] {
        //       auto [xau, xag] = model.step(dt);
        //       pub.publish(mms::FairValue{mms::dollars_to_ticks(xau), mms::dollars_to_ticks(xag)});
        //       if (++n % 20 == 0) mms::log::info("XAU ", xau, " XAG ", xag, " ratio ", xau / xag);
        //   });
        //   loop.run();
        (void)kSecondsPerYear;
        mms::log::error("fundamentals main() not implemented yet");
        return 1;
    } catch (const std::exception& e) {
        mms::log::error("fatal: ", e.what());
        return 1;
    }
}
