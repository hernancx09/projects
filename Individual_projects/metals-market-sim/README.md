# Metals Market Sim (C++)

A **virtual distributed exchange** for gold (XAU) and silver (XAG). It's written in
C++20 on raw Linux sockets and runs as Docker containers on virtual networks.
Trading bots compete in it, and eventually **your own trading system** does too.

This repo is a **scaffold**. The structure, interfaces, and explanations are here.
The implementation is not. Everywhere you need to write code is marked `TODO`, and
every unwritten function calls `not_implemented()`, which throws an error naming
that exact function. Run something, and it tells you what to build next.

---

## How the TODOs are organized

Each TODO is tagged with the phase it belongs to:

```
// TODO(P1): ...        <- do these first
// TODO(P2): ...
// TODO(STRETCH): ...   <- optional, once everything works
```

```bash
grep -rn "TODO(P1)" --include=*.cpp --include=*.hpp .     # everything for phase 1
grep -rln "not_implemented()" --include=*.cpp --include=*.hpp .   # files with stubs left
```

**Already implemented for you** (plumbing, not lessons): `common/log.*`, `common/todo.hpp`,
the message structs in `common/messages.hpp`, `ByteWriter::u8/u16`, and `write_body(Login)`
as worked examples.

---

## Toolchain (Windows -> Linux)

The code uses the **Linux** socket API (`epoll`, `timerfd`, POSIX sockets), so it
builds and runs in Linux: WSL2 for your dev loop, Docker for the full system.
Your MSYS2 g++ produces Windows programs and won't work for the networking code.

1. **WSL2 + Ubuntu** (PowerShell as admin): `wsl --install -d Ubuntu-24.04`, then reboot.
2. **Inside Ubuntu**: `sudo apt update && sudo apt install -y build-essential cmake gdb manpages-dev`
3. **Docker Desktop** for Windows, with "Use the WSL 2 based engine" enabled.
   Check it with `docker --version` and `docker compose version`.
4. **VS Code**: install the *WSL* extension, then from Ubuntu run `code .` in this folder.
   Add the *C/C++* (or *clangd*) and *CMake Tools* extensions inside the WSL window.
5. **Wireshark** on Windows (optional, but highly recommended for looking at packets).

Tip: WSL can reach this folder at `/mnt/c/Users/<you>/Desktop/projects/...`, but builds
are much faster if you clone the repo into your Linux home (`~/`).

### Build & test (in WSL)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DMMS_SANITIZERS=ON   # once
cmake --build build -j                                             # every change
ctest --test-dir build --output-on-failure                         # unit tests
./build/bin/unit_tests --gtest_filter='Protocol.*'                 # one test group
```

Binaries land in `build/bin/`: `engine`, `gateway`, `load_balancer`, `fundamentals`,
`bots`, `my_trader`, `leaderboard`, `unit_tests`.

**Keep `-DMMS_SANITIZERS=ON` while developing.** AddressSanitizer catches buffer overflows
and use-after-free the moment they happen. Raw byte-buffer networking code is exactly where
those bugs live.

---

## Architecture

```
                         trading_net  (the "public internet" of the sim)
   +-------------+   +-------------+
   |    bots     |   |  my_trader  |   <- same client SDK (common/client.*), same rules
   +------+------+   +------+------+
          |  TCP orders      |
          +--------+---------+
                   v
          +-----------------+
          |  load_balancer  |  :9000   L4 TCP proxy (round robin / least-conn / ip-hash)
          +--------+--------+          + health checks on their own thread
   ----------------|------------------------------ core_net (private, internal only)
         +---------+----------+
         v         v          v
    +---------+ +---------+ +---------+
    |gateway1 | |gateway2 | |gateway3 |  :9100   stateless: login, validate, rate-limit,
    +----+----+ +----+----+ +----+----+          route each order by SYMBOL
         +-----------+-----------+
               |             |
               v             v
        +------------+ +------------+
        | engine_xau | | engine_xag |   :9200 orders (TCP)    one matching engine per
        +-----+------+ +-----+------+   :9300 market data     symbol = SHARDING
              |              |          :9500 admin text
              +---- UDP market data (quotes, trades) ---->  bots / my_trader
                                                            (bypasses the LB, like
   +--------------+                                          real exchanges do)
   | fundamentals |  :9400  UDP "true value" of gold & silver (correlated random walk)
   +--------------+         -> only the informed bot sees it
```

### Inside every service: the same pattern

```
main.cpp  -> reads env vars, builds objects, calls loop.run()
EventLoop -> epoll: "which sockets are ready?" -> calls your callback -> repeat
             ONE thread, no locks (except the LB's health checker thread)
TcpConnection -> recv bytes -> FrameBuffer -> whole Messages -> your handler
                 send(Message) -> encode -> buffer -> write when the socket allows
```

### What each piece teaches

| Piece | You learn |
|---|---|
| `common/protocol.*` | TCP is a **byte stream**: build your own message boundaries. Big-endian encoding, why you can't `memcpy` structs onto the wire. |
| `common/socket.*`, `event_loop.*` | The real POSIX API: `socket/bind/listen/accept`, non-blocking I/O, `epoll`, `timerfd`, `EAGAIN`, `SIGPIPE`, `TIME_WAIT`. |
| `common/connection.*` | Partial reads and writes, output buffering, backpressure, Nagle's algorithm. |
| `load_balancer/` | L4 proxying, balancing algorithms, health checks with hysteresis, one real thread + `std::atomic`. |
| `gateway/` | Stateless tiers, multiplexing many clients over one upstream connection, rate limiting, never trusting input. |
| `engine/` | Sharding, single-writer determinism, price-time priority, `std::map` vs `std::list` iterator stability. |
| `common/udp_feed.*` + client | Datagrams, sequence numbers, gap detection, MTU and fragmentation, why exchanges use multicast. |
| `tools/netem.sh` | Inject latency, loss, and reordering, then watch TCP hide it and UDP expose it. |

---

## Folder map

```
CMakeLists.txt          one build file for everything (libraries + executables + tests)
Dockerfile              multi-stage: compile in a big image, ship binaries in a small one
docker-compose.yml      every service, network, and port

common/                 shared library
  messages.hpp            message structs + std::variant Message        <- read first
  protocol.hpp/.cpp       binary wire format, FrameBuffer, UDP datagrams <- implement first
  socket.hpp/.cpp         RAII Fd + POSIX socket helpers
  event_loop.hpp/.cpp     epoll + timerfd
  connection.hpp/.cpp     framed non-blocking TCP connection
  udp_feed.hpp/.cpp       UDP publisher (engines, fundamentals)
  client.hpp/.cpp         the SDK every trader uses
  config.*, log.*, todo.hpp

load_balancer/          strategies, health (thread), proxy, main
gateway/                gateway, token_bucket.hpp, main
engine/                 order_book (matching), accounts (P&L, risk), engine (server), main
fundamentals/           gbm (correlated random walk), main
bots/                   base_bot + market_maker, noise_trader, momentum, informed_trader, main
my_trader/              YOUR system: trader, risk, main
tools/                  netem.sh (network chaos), leaderboard.cpp
tests/                  GoogleTest specs that fail until you implement things
```

---

## Roadmap

Each phase should **run** before you start the next one.

### Phase 0: Setup
- [ ] Toolchain above. `cmake --build build` succeeds (everything compiles from day one)
- [ ] `TODO(P0)` in `common/config.cpp`

### Phase 1: Wire protocol + your first socket (WSL, no Docker)
- [ ] `common/protocol.cpp` until `./build/bin/unit_tests --gtest_filter='Protocol.*'` passes
- [ ] `TODO(P1)` in `common/socket.cpp` (the `Fd` RAII class first!)
- [ ] Simplest possible server: `engine/main.cpp` with a **blocking** `accept()` + `recv()` loop that echoes frames back.
      Skip the event loop for now and just get bytes moving.
- [ ] Capture it: `sudo tcpdump -i lo -X port 9200`. Find the 3-way handshake and your 2-byte length prefix.

### Phase 2: Event loop, load balancer, gateways (Docker)
- [ ] `common/event_loop.cpp`, `common/connection.cpp`. Convert the engine to use them.
- [ ] `load_balancer/` (round robin first, health checks second), `gateway/`
- [ ] `docker compose up --build engine_xau engine_xag gateway1 gateway2 gateway3 load_balancer`
- [ ] Open several connections. Confirm in the LB logs that they spread across gateways.
- [ ] `docker compose stop gateway2`. Watch the health checker mark it DOWN and route around it.
- [ ] `ctest`: Strategies tests pass

### Phase 3: Matching engine
- [ ] `engine/order_book.cpp` until the OrderBook tests pass
- [ ] `engine/accounts.cpp`, `Engine::handle_new_order/handle_cancel`
- [ ] Gateway validation + `TokenBucket` (tests pass)
- [ ] End to end: an order sent to `localhost:9000` matches, and a Fill comes back

### Phase 4: UDP market data
- [ ] `common/udp_feed.cpp`, the UDP half of `common/client.cpp`, the datagram functions in protocol.cpp
- [ ] Gap detection + latency stats
- [ ] `docker compose exec engine_xau bash /app/tools/netem.sh loss 5% <iface>` and watch gaps appear
      (the engine has two interfaces, so run `ip addr` inside it to find the one on trading_net)

### Phase 5: Fundamentals + bots
- [ ] `fundamentals/`: correlated GBM
- [ ] `bots/base_bot.cpp`, then **market maker + noise trader** (you need both for a living market)
- [ ] Momentum and informed traders
- [ ] `docker compose up --build`: the whole market runs on its own

### Phase 6: Compete
- [ ] Engine admin endpoint + `tools/leaderboard.cpp`
- [ ] `my_trader/`: **risk manager first**, then a strategy (ideas in `my_trader/trader.hpp`)
- [ ] `docker compose --profile compete up --build`, then `docker compose exec bots /app/bin/leaderboard`
- [ ] Beat the bots. Then make the bots smarter, and beat them again.

### Stretch ideas
- Benchmark the order book (millions of random orders), then switch to a flat price array + object pool
- Measure order round-trip latency end to end: with and without `TCP_NODELAY`, Debug vs Release
- Replace your LB with **HAProxy** or **nginx** `stream` mode and compare
- Gap *recovery*: a TCP retransmit service backed by a ring buffer of recent datagrams
- Primary/backup engine replication with failover
- Real UDP multicast (macvlan network)
- `io_uring` instead of epoll

---

## Running

```bash
docker compose up --build                       # whole market (without my_trader)
docker compose --profile compete up --build     # ... with my_trader
docker compose up --build engine_xau            # one service (easier while building)
docker compose logs -f load_balancer            # one service's logs
docker compose exec gateway1 bash               # shell inside (ping, dig, ss -tunap, tcpdump, tc)
docker compose down                             # tear down
```

Run a service locally in WSL (handy in phase 1):

```bash
SYMBOL=XAU ./build/bin/engine
```

## Ports

| Service | Port | Proto | Purpose |
|---|---|---|---|
| load_balancer | 9000 | TCP | client entry point (also published to your PC as localhost:9000) |
| load_balancer | 9001 | TCP | stats text (stretch) |
| gateway* | 9100 | TCP | LB -> gateway |
| engine_* | 9200 | TCP | gateway -> engine (orders) |
| engine_* | 9300 | UDP | market data |
| fundamentals | 9400 | UDP | fair value feed |
| engine_* | 9500 | TCP | admin text (leaderboard) |

## Wire format at a glance

```
TCP frame:     [len u16 BE][type u8][body...]           len = 1 + body size
UDP datagram:  [seq u64][send_ns u64][ver u8][one TCP-style frame]
Prices:        int64 ticks, 1 tick = $0.01   ($2400.15 -> 240015)
Accounts:      16-byte zero-padded fixed-width strings
```

## Keywords to look up as you go

`TCP byte stream framing`, `network byte order`, `RAII`, `move semantics`, `epoll level vs edge triggered`,
`EAGAIN`, `SIGPIPE MSG_NOSIGNAL`, `TIME_WAIT SO_REUSEADDR`, `Nagle TCP_NODELAY`, `non-blocking connect EINPROGRESS`,
`backpressure`, `head-of-line blocking`, `L4 vs L7 load balancing`, `consistent hashing`, `std::atomic memory order`,
`MTU IP fragmentation`, `UDP gap detection`, `multicast`, `price-time priority`, `iterator invalidation`,
`geometric Brownian motion`, `Cholesky`, `inventory skew`, `adverse selection`, `gold/silver ratio`, `pairs trading`.
