#pragma once
// =============================================================================
// The hidden "true value" of gold and silver: correlated Geometric Brownian Motion.
//
// Without this, prices would come only from bots trading with each other and
// would wander aimlessly or freeze. This gives the market something REAL to
// discover. Informed bots see it. Everyone else only sees prices.
//
// GBM, the textbook price model. Each step:
//
//     S_next = S * exp( (mu - 0.5*sigma^2)*dt + sigma*sqrt(dt)*Z )
//
//   mu     drift per year (use 0 for a fair game)
//   sigma  volatility per year (0.15 = 15%)
//   dt     step size IN YEARS
//   Z      standard normal random number
// exp() keeps prices positive. The -0.5*sigma^2 term stops the average from
// drifting upward. (Look up "Ito correction" if you're curious.)
//
// CORRELATION rho between the two (gold and silver usually move together):
//   Z1 = normal()
//   Z2 = rho*Z1 + sqrt(1 - rho^2) * normal()
// That's a 2x2 Cholesky decomposition.
//
// WHY YOU CARE: with high correlation the gold/silver RATIO stays fairly stable
// (~80 here). If MARKET prices break that relationship, that's a pairs trade. ;)
// =============================================================================
#include <cstdint>
#include <random>
#include <utility>

namespace mms {

class CorrelatedGbm {
public:
    CorrelatedGbm(double xau_start, double xag_start, double xau_vol, double xag_vol, double rho,
                  double mu = 0.0, uint64_t seed = 42);

    // Advance one step, return {xau, xag} in dollars.
    std::pair<double, double> step(double dt_years);

    double xau() const { return xau_; }
    double xag() const { return xag_; }

private:
    double xau_, xag_;
    double xau_vol_, xag_vol_;
    double rho_, mu_;
    // A SEEDED generator makes runs REPRODUCIBLE: same seed, same price path.
    // That's essential for comparing two versions of your strategy fairly.
    std::mt19937_64 rng_;
    std::normal_distribution<double> normal_{0.0, 1.0};

    // TODO(STRETCH): JUMPS. With small probability per step, apply a sudden
    //   +/-2-5% move ("news"). Do the bots cope? Does your strategy?
    // TODO(STRETCH): make the RATIO mean-revert (Ornstein-Uhlenbeck) so pairs
    //   trading reliably works, then see if your trader finds it.
};

}  // namespace mms
