#include "common/log.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <mutex>

namespace mms::log {

namespace {
std::string g_service = "?";
Level g_level = Level::Info;
std::mutex g_mutex;  // the LB's health-check thread logs too, so writes must not interleave
}  // namespace

void init(std::string service_name) {
    g_service = std::move(service_name);
    if (const char* lv = std::getenv("LOG_LEVEL")) {
        std::string s = lv;
        if (s == "DEBUG") g_level = Level::Debug;
        else if (s == "WARN") g_level = Level::Warn;
        else if (s == "ERROR") g_level = Level::Error;
        else g_level = Level::Info;
    }
}

bool enabled(Level level) { return level >= g_level; }

void write(Level level, const std::string& message) {
    using namespace std::chrono;
    const auto now = system_clock::now();
    const std::time_t t = system_clock::to_time_t(now);
    const long long ms = duration_cast<milliseconds>(now.time_since_epoch()).count() % 1000;
    std::tm tm{};
    gmtime_r(&t, &tm);  // POSIX, thread-safe version of gmtime
    char clock[16];
    std::strftime(clock, sizeof clock, "%H:%M:%S", &tm);

    static constexpr const char* kNames[] = {"DEBUG", "INFO", "WARN", "ERROR"};
    std::lock_guard lock(g_mutex);
    std::fprintf(stderr, "%s.%03lld %-5s [%s] %s\n", clock, ms, kNames[static_cast<int>(level)],
                 g_service.c_str(), message.c_str());
}

}  // namespace mms::log
