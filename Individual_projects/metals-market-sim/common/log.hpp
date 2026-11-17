#pragma once
// -----------------------------------------------------------------------------
// Tiny logger. This one is already implemented: it's plumbing, not a lesson,
// and you'll need logs to debug everything else.
//
// Usage:
//   mms::log::init("engine_xau");            // once at startup (reads LOG_LEVEL env)
//   mms::log::info("order ", id, " filled ", qty, " @ ", price);
//
// Output (to stderr, which `docker compose logs` collects):
//   14:02:07.123 INFO  [engine_xau] order 17 filled 5 @ 240015
//
// The millisecond timestamps let you follow ONE order through
// LB -> gateway -> engine across `docker compose logs -f` output.
// -----------------------------------------------------------------------------
#include <sstream>
#include <string>

namespace mms::log {

enum class Level { Debug = 0, Info = 1, Warn = 2, Error = 3 };

void init(std::string service_name);  // also reads LOG_LEVEL=DEBUG|INFO|WARN|ERROR
bool enabled(Level level);
void write(Level level, const std::string& message);

// Fold expression: (os << ... << args) expands to os << a1 << a2 << a3 ...
template <class... Args>
std::string concat(const Args&... args) {
    std::ostringstream os;
    (os << ... << args);
    return os.str();
}

template <class... Args> void debug(const Args&... a) { if (enabled(Level::Debug)) write(Level::Debug, concat(a...)); }
template <class... Args> void info(const Args&... a)  { if (enabled(Level::Info))  write(Level::Info,  concat(a...)); }
template <class... Args> void warn(const Args&... a)  { if (enabled(Level::Warn))  write(Level::Warn,  concat(a...)); }
template <class... Args> void error(const Args&... a) { if (enabled(Level::Error)) write(Level::Error, concat(a...)); }

}  // namespace mms::log
