#include "common/config.hpp"

#include <cstdlib>
#include <stdexcept>

#include "common/todo.hpp"

namespace mms::config {

std::string env_str(const char* name, std::optional<std::string> fallback) {
    // TODO(P0): const char* v = std::getenv(name);
    //   if v -> return it; else if fallback -> return *fallback;
    //   else throw std::runtime_error(std::string("missing env var ") + name)
    not_implemented();
}

int env_int(const char* name, std::optional<int> fallback) {
    // TODO(P0): like env_str, then std::stoi. stoi throws on garbage ("abc"). Let it,
    //   but maybe catch and rethrow with the variable name so the error is useful.
    not_implemented();
}

double env_double(const char* name, std::optional<double> fallback) {
    // TODO(P0): std::stod
    not_implemented();
}

HostPort parse_addr(std::string_view text) {
    // TODO(P0): find the LAST ':' with text.rfind(':'); host = before, port = after.
    //   Validate: port in 1..65535 (uint16_t would silently wrap 70000 -> 4464!).
    not_implemented();
}

std::vector<HostPort> parse_addr_list(std::string_view text) {
    // TODO(P0): split on ',' and parse_addr each piece. Skip empty pieces.
    not_implemented();
}

std::map<std::string, HostPort> parse_symbol_map(std::string_view text) {
    // TODO(P0): split on ',', then each "XAU=engine_xau:9200" on '='.
    not_implemented();
}

}  // namespace mms::config
