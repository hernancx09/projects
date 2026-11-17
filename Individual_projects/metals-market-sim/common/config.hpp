#pragma once
// -----------------------------------------------------------------------------
// Configuration from ENVIRONMENT VARIABLES.
//
// docker-compose.yml sets them under `environment:`. The same binary then
// runs as engine_xau or engine_xag, or with a different LB strategy, without
// recompiling. (This is the "12-factor app" approach.)
//
// Running locally in WSL:   SYMBOL=XAU ./build/bin/engine
// -----------------------------------------------------------------------------
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mms::config {

struct HostPort {
    std::string host;
    uint16_t port = 0;
    bool operator==(const HostPort&) const = default;
};

// Read an env var. If it's missing and there's no default, throw a clear
// error. Failing loudly at startup beats a confusing crash later.
std::string env_str(const char* name, std::optional<std::string> fallback = std::nullopt);
int env_int(const char* name, std::optional<int> fallback = std::nullopt);
double env_double(const char* name, std::optional<double> fallback = std::nullopt);

// "engine_xau:9200" -> {"engine_xau", 9200}
HostPort parse_addr(std::string_view text);

// "gateway1:9100,gateway2:9100" -> [{gateway1, 9100}, {gateway2, 9100}]
std::vector<HostPort> parse_addr_list(std::string_view text);

// "XAU=engine_xau:9200,XAG=engine_xag:9200" -> {"XAU": {...}, "XAG": {...}}
std::map<std::string, HostPort> parse_symbol_map(std::string_view text);

}  // namespace mms::config
