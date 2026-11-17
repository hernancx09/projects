#include "fundamentals/gbm.hpp"

#include <cmath>

#include "common/todo.hpp"

namespace mms {

CorrelatedGbm::CorrelatedGbm(double xau_start, double xag_start, double xau_vol, double xag_vol, double rho,
                             double mu, uint64_t seed)
    : xau_(xau_start), xag_(xag_start), xau_vol_(xau_vol), xag_vol_(xag_vol), rho_(rho), mu_(mu), rng_(seed) {}

std::pair<double, double> CorrelatedGbm::step(double dt_years) {
    // TODO(P5):
    //   double z1 = normal_(rng_);
    //   double z2 = rho_ * z1 + std::sqrt(1.0 - rho_ * rho_) * normal_(rng_);
    //   xau_ *= std::exp((mu_ - 0.5 * xau_vol_ * xau_vol_) * dt_years + xau_vol_ * std::sqrt(dt_years) * z1);
    //   xag_ *= ... same with z2
    //   return {xau_, xag_};
    // Sanity check: run 1,000,000 steps and confirm the measured correlation of
    //   the log-returns is close to rho. (A great first unit test to write yourself.)
    not_implemented();
}

}  // namespace mms
