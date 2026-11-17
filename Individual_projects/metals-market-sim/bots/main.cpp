// Entry point for the bots container: runs MANY bots in ONE process, on ONE EventLoop.
//
//   Env: LB_ADDR, MD_ADDRS, FUNDAMENTALS_ADDR, BOTS, SEED
//   BOTS="market_maker:2,noise_trader:5" -> accounts market_maker_1, market_maker_2, noise_trader_1, ...
//
// Each bot gets its OWN ExchangeClient = its own TCP connection through the LB.
// With 10 bots and 3 gateways, watch the LB logs spread them across gateways.
#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "bots/base_bot.hpp"
#include "bots/informed_trader.hpp"
#include "bots/market_maker.hpp"
#include "bots/momentum.hpp"
#include "bots/noise_trader.hpp"
#include "common/config.hpp"
#include "common/event_loop.hpp"
#include "common/log.hpp"

namespace {

// A FACTORY: bot type name -> a function that builds one.
// Adding a new bot type = one new line here.
using Factory = std::function<std::unique_ptr<mms::BaseBot>(std::string, mms::EventLoop&,
                                                            std::unique_ptr<mms::ExchangeClient>, uint64_t)>;

template <class Bot>
Factory make_factory(std::chrono::milliseconds tick) {
    return [tick](std::string name, mms::EventLoop& loop, std::unique_ptr<mms::ExchangeClient> client, uint64_t seed) {
        return std::make_unique<Bot>(std::move(name), loop, std::move(client), seed, tick);
    };
}

// "market_maker:2,noise_trader:5" -> {{"market_maker", 2}, {"noise_trader", 5}}
std::vector<std::pair<std::string, int>> parse_bot_spec(const std::string& spec) {
    // TODO(P5)
    mms::not_implemented();
}

}  // namespace

int main() {
    mms::log::init("bots");
    try {
        const std::map<std::string, Factory> factories = {
            {"market_maker", make_factory<mms::MarketMaker>(std::chrono::milliseconds(500))},
            {"noise_trader", make_factory<mms::NoiseTrader>(std::chrono::milliseconds(1000))},
            {"momentum", make_factory<mms::Momentum>(std::chrono::milliseconds(1000))},
            {"informed_trader", make_factory<mms::InformedTrader>(std::chrono::milliseconds(2000))},
        };

        // TODO(P5):
        //   auto lb   = mms::config::parse_addr(mms::config::env_str("LB_ADDR"));
        //   auto md   = mms::config::parse_symbol_map(mms::config::env_str("MD_ADDRS"));
        //   auto fund = mms::config::parse_addr(mms::config::env_str("FUNDAMENTALS_ADDR"));
        //   mms::EventLoop loop;
        //   std::vector<std::unique_ptr<mms::BaseBot>> bots;
        //   for (auto& [type, count] : parse_bot_spec(mms::config::env_str("BOTS")))
        //     for (int i = 1; i <= count; ++i) {
        //       auto name = type + "_" + std::to_string(i);
        //       // ONLY informed traders get the fundamentals feed:
        //       auto client = std::make_unique<mms::ExchangeClient>(loop, name, lb, md,
        //           type == "informed_trader" ? std::optional(fund) : std::nullopt);
        //       bots.push_back(factories.at(type)(name, loop, std::move(client), seed + bots.size()));
        //     }
        //   for (auto& b : bots) b->start();
        //   loop.run();
        (void)factories;
        mms::log::error("bots main() not implemented yet");
        return 1;
    } catch (const std::exception& e) {
        mms::log::error("fatal: ", e.what());
        return 1;
    }
}
