# =============================================================================
# MULTI-STAGE build: one image for every service.
#
# Stage 1 ("build") has the compiler, CMake, headers: big (~500MB).
# Stage 2 (runtime) gets ONLY the compiled binaries + some network tools.
# The compiler never ships. That's the standard pattern for C++/Go/Rust images.
#
# docker-compose.yml picks which binary each container runs with `command:`.
# TODO(STRETCH): look at the final size with `docker images`. Then try a
#   static build (-static) on a `scratch` or distroless base image.
# =============================================================================

# ---------------------------------------------------------------- stage 1
FROM ubuntu:24.04 AS build

# build-essential = g++, make, libc headers. Ubuntu 24.04 ships g++ 13 (full C++20).
RUN apt-get update \
 && apt-get install -y --no-install-recommends build-essential cmake ca-certificates \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

# Release = optimized (-O3). Tests are off here: they're for your dev loop, not the image.
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DMMS_BUILD_TESTS=OFF \
 && cmake --build build -j"$(nproc)"

# ---------------------------------------------------------------- stage 2
FROM ubuntu:24.04

# Networking tools so you can poke at the network from INSIDE a container:
#   iproute2        -> `ip addr`, `ss -tunap` (list sockets), `tc` (netem fault injection)
#   tcpdump         -> capture packets: `tcpdump -i eth0 -n -X port 9100`
#   iputils-ping    -> `ping engine_xau`
#   netcat-openbsd  -> `nc -v engine_xau 9500` raw TCP/UDP poking
#   dnsutils        -> `dig engine_xau` to see Docker's internal DNS at work
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
        iproute2 tcpdump iputils-ping netcat-openbsd dnsutils \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=build /src/build/bin/ /app/bin/
COPY tools/netem.sh /app/tools/netem.sh

# No CMD here on purpose: every service sets `command:` in docker-compose.yml.
