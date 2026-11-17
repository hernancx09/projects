// =============================================================================
// Leaderboard: who's winning?
//
// Connects to each engine's admin port, reads the per-account text snapshot,
// combines both symbols per account, and prints a table sorted by P&L:
//
//   RANK  ACCOUNT            XAU POS  XAG POS    P&L ($)   VOLUME
//      1  me                      12      -40     1234.56      980
//      2  market_maker_1          -3       10      402.10     5120
//      ...
//
// Run it from inside the network (the engines' admin ports aren't published to your PC):
//   docker compose exec bots /app/bin/leaderboard
//   (or: docker compose run --rm bots /app/bin/leaderboard)
//
//   Env: ENGINES_ADMIN="XAU=engine_xau:9500,XAG=engine_xag:9500"
// =============================================================================
#include <exception>
#include <map>
#include <string>

#include "common/config.hpp"
#include "common/log.hpp"
#include "common/socket.hpp"
#include "common/todo.hpp"

namespace {

struct Row {
    std::map<std::string, int> position;  // symbol -> position
    double pnl_ticks = 0;
    long long volume = 0;
};

// Connect, read until the server closes the connection (recv returns 0), return everything.
std::string fetch(const mms::config::HostPort& hp) {
    // TODO(P6): net::tcp_connect + recv loop into a std::string
    mms::not_implemented();
}

}  // namespace

int main() {
    mms::log::init("leaderboard");
    try {
        // TODO(P6):
        //   auto engines = mms::config::parse_symbol_map(mms::config::env_str("ENGINES_ADMIN", "XAU=engine_xau:9500,XAG=engine_xag:9500"));
        //   std::map<std::string, Row> rows;
        //   for each engine: parse fetch() output (std::istringstream, line by line) into rows
        //   sort by pnl, print with std::printf("%4d  %-16s ...")
        // TODO(STRETCH): refresh every 2 seconds and redraw (print "\033[2J\033[H" to clear the terminal).
        (void)&fetch;
        mms::log::error("leaderboard not implemented yet");
        return 1;
    } catch (const std::exception& e) {
        mms::log::error("fatal: ", e.what());
        return 1;
    }
}
