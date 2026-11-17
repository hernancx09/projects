# =============================================================================
# Top-level build file for the whole project.
#
# CMake doesn't compile anything itself. It GENERATES build files (Makefiles
# or Ninja) for your compiler. Two steps:
#
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug     # configure (once, or when this file changes)
#   cmake --build build -j                           # compile
#   ctest --test-dir build --output-on-failure       # run unit tests
#
# Executables land in build/bin/.
#
# This code targets LINUX (POSIX sockets + epoll). Build it inside WSL2 or let
# the Dockerfile build it. See README.md, "Toolchain".
#
# Structure: each component is a STATIC LIBRARY (the logic) + a tiny
# EXECUTABLE (main.cpp that reads config and starts it). The split exists so
# the unit tests can link the logic without the main().
# =============================================================================
cmake_minimum_required(VERSION 3.20)
project(metals_market_sim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)              # we use std::span, std::variant, designated initializers, <=> ...
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)           # -std=c++20, not -std=gnu++20
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # writes build/compile_commands.json so VS Code/clangd understand the code
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT CMAKE_BUILD_TYPE)
  # Debug = no optimization, full debug info (use while building)
  # Release = -O3 (use when you measure latency!)
  # RelWithDebInfo = optimized but still debuggable (a good default)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

option(MMS_BUILD_TESTS "Build unit tests (downloads GoogleTest)" ON)
option(MMS_SANITIZERS "Build with AddressSanitizer + UndefinedBehaviorSanitizer" OFF)

# Warnings are free bug reports. The two -Wno-unused-* flags only exist to keep
# the stubs quiet (stub functions ignore their parameters, and some helpers
# aren't called yet).
# TODO(STRETCH): remove both once everything is implemented, and consider -Werror.
add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function)

# Sanitizers catch memory bugs (use-after-free, buffer overflow, ...) at RUNTIME.
# Turn this ON while developing:  cmake -S . -B build -DMMS_SANITIZERS=ON
# Networking code with raw buffers is exactly where these bugs live.
if(MMS_SANITIZERS)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

find_package(Threads REQUIRED)   # the LB's health checker runs on its own std::thread

# ----------------------------------------------------------------- common
add_library(mms_common STATIC
  common/config.cpp
  common/log.cpp
  common/protocol.cpp
  common/socket.cpp
  common/event_loop.cpp
  common/connection.cpp
  common/udp_feed.cpp
  common/client.cpp
)
# PUBLIC include dir = the project root, so everyone writes #include "common/protocol.hpp"
target_include_directories(mms_common PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(mms_common PUBLIC Threads::Threads)

# ----------------------------------------------------------- load balancer
add_library(mms_lb STATIC
  load_balancer/strategies.cpp
  load_balancer/health.cpp
  load_balancer/load_balancer.cpp
)
target_link_libraries(mms_lb PUBLIC mms_common)
add_executable(load_balancer load_balancer/main.cpp)
target_link_libraries(load_balancer PRIVATE mms_lb)

# ----------------------------------------------------------------- gateway
add_library(mms_gateway STATIC gateway/gateway.cpp)
target_link_libraries(mms_gateway PUBLIC mms_common)
add_executable(gateway gateway/main.cpp)
target_link_libraries(gateway PRIVATE mms_gateway)

# ------------------------------------------------------------------ engine
add_library(mms_engine STATIC
  engine/order_book.cpp
  engine/accounts.cpp
  engine/engine.cpp
)
target_link_libraries(mms_engine PUBLIC mms_common)
add_executable(engine engine/main.cpp)
target_link_libraries(engine PRIVATE mms_engine)

# ------------------------------------------------------------ fundamentals
add_library(mms_fundamentals STATIC fundamentals/gbm.cpp)
target_link_libraries(mms_fundamentals PUBLIC mms_common)
add_executable(fundamentals fundamentals/main.cpp)
target_link_libraries(fundamentals PRIVATE mms_fundamentals)

# -------------------------------------------------------------------- bots
add_library(mms_bots STATIC bots/base_bot.cpp)
target_link_libraries(mms_bots PUBLIC mms_common)
add_executable(bots bots/main.cpp)
target_link_libraries(bots PRIVATE mms_bots)

# --------------------------------------------------------------- my_trader
add_executable(my_trader
  my_trader/main.cpp
  my_trader/trader.cpp
  my_trader/risk.cpp
)
target_link_libraries(my_trader PRIVATE mms_bots)

# ------------------------------------------------------------------- tools
add_executable(leaderboard tools/leaderboard.cpp)
target_link_libraries(leaderboard PRIVATE mms_common)

# ------------------------------------------------------------------- tests
if(MMS_BUILD_TESTS)
  enable_testing()
  include(FetchContent)
  # FetchContent downloads GoogleTest at configure time and builds it with
  # the project. No system install needed.
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
  )
  FetchContent_MakeAvailable(googletest)

  add_executable(unit_tests
    tests/test_protocol.cpp
    tests/test_order_book.cpp
    tests/test_token_bucket.cpp
    tests/test_strategies.cpp
  )
  target_link_libraries(unit_tests PRIVATE mms_engine mms_gateway mms_lb GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(unit_tests)
endif()
