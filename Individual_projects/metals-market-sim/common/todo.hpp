#pragma once
// -----------------------------------------------------------------------------
// not_implemented(): every stub in this project calls it.
//
// It throws an exception that names the exact function, file, and line, so
// when you run a service you immediately see the NEXT thing to build:
//
//   terminate called after throwing an instance of 'std::logic_error'
//     what():  TODO not implemented: mms::protocol::encode_frame(...) (common/protocol.cpp:42)
//
// [[noreturn]] tells the compiler this never returns normally, so a stub like
//     int foo() { not_implemented(); }
// compiles without a "missing return" warning.
//
// When you implement a function, delete its not_implemented() call.
// When `grep -rn not_implemented --include=*.cpp --include=*.hpp .` only finds
// this file, you're done.
// -----------------------------------------------------------------------------
#include <source_location>
#include <stdexcept>
#include <string>

namespace mms {

[[noreturn]] inline void not_implemented(std::source_location loc = std::source_location::current()) {
    throw std::logic_error(std::string("TODO not implemented: ") + loc.function_name() + " (" +
                           loc.file_name() + ":" + std::to_string(loc.line()) + ")");
}

}  // namespace mms
