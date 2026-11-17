#pragma once
// -----------------------------------------------------------------------------
// Message types: the "language" every service speaks.
//
// These are plain C++ structs. They are NOT the wire format. protocol.cpp
// converts them to/from bytes field by field. Why not just memcpy a struct
// onto the socket? Three reasons:
//   1. PADDING: the compiler inserts invisible gap bytes between fields for
//      alignment. Their layout depends on compiler and flags.
//   2. ENDIANNESS: x86 stores ints little-endian. The network convention is
//      big-endian. memcpy sends whatever the CPU uses.
//   3. std::string holds a POINTER to heap memory. Sending the pointer is
//      meaningless to the other process.
// (Real low-latency systems DO send packed structs, but with
// __attribute__((packed)), fixed-width types, and explicit byte-order
// fields. TODO(STRETCH): try it and measure.)
//
// PRICES ARE INTEGER TICKS, NOT DOUBLES
// -------------------------------------
// 0.1 + 0.2 != 0.3 in floating point. If two orders at "$2400.15" become
// slightly different doubles, they won't match. Exchanges use integers:
// 1 tick = $0.01, so $2400.15 == 240015. Convert to dollars only for display.
//
// Flow of one order:
//   client --NewOrder--> LB --> gateway --> engine
//   client <--OrderAck / Reject------------ engine
//   client <--Fill (0..many, maybe later)-- engine
//   everyone <--Trade + Quote (UDP)-------- engine
// -----------------------------------------------------------------------------
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

namespace mms {

using Price = int64_t;           // ticks
using Qty = int32_t;             // ounces. SIGNED on purpose: positions go negative, and unsigned math underflows silently
using OrderId = uint64_t;        // assigned by the engine
using ClientOrderId = uint64_t;  // chosen by the client, so it can match replies to requests

constexpr double kTickSize = 0.01;  // dollars per tick

inline double ticks_to_dollars(Price p) { return static_cast<double>(p) * kTickSize; }
inline Price dollars_to_ticks(double d) { return std::llround(d / kTickSize); }  // llround, NOT a cast: (long)(2400.15/0.01) can be 240014!

// ---------------------------------------------------------------- enums
// `enum class` + an explicit underlying type = a fixed 1-byte wire value.
enum class Symbol : uint8_t { XAU = 0, XAG = 1 };  // gold, silver
constexpr std::array<Symbol, 2> kAllSymbols{Symbol::XAU, Symbol::XAG};

enum class Side : uint8_t { Buy = 0, Sell = 1 };
enum class OrderType : uint8_t {
    Limit = 0,   // trade at `price` or better, rest on the book if not fully filled
    Market = 1,  // trade now at whatever is available, never rests
    // TODO(STRETCH): IOC (immediate-or-cancel), PostOnly (reject if it would trade)
};

enum class RejectReason : uint8_t {
    Unknown = 0,
    NotLoggedIn,
    DuplicateLogin,
    BadMessage,
    UnknownSymbol,
    InvalidQty,
    InvalidPrice,
    RateLimited,
    RiskLimit,
    UnknownOrder,
    EngineUnavailable,
};

// The first byte of every message body on the wire.
enum class MsgType : uint8_t {
    // client -> exchange (TCP, via LB + gateway)
    Login = 1,
    NewOrder = 2,
    Cancel = 3,
    // exchange -> client (TCP)
    LoginAck = 10,
    OrderAck = 11,
    CancelAck = 12,
    Reject = 13,
    Fill = 14,
    // market data (UDP)
    Subscribe = 20,  // client -> engine: "send the feed to the address this packet came from"
    Quote = 21,      // top of book
    Trade = 22,      // a trade happened (anonymous! no account names in public data)
    Heartbeat = 23,  // "still alive, last seq was N"
    // fundamentals (UDP)
    FairValue = 30,
    // health checks
    Ping = 40,
    Pong = 41,
};

// --------------------------------------------------------------- messages
// Each struct carries its MsgType as `kType` so generic code can find it
// (see type_of() below). `operator== = default` (C++20) generates
// field-by-field equality. The tests use it.

struct Login {
    static constexpr MsgType kType = MsgType::Login;
    std::string account;  // max 16 chars on the wire
    bool operator==(const Login&) const = default;
};

struct LoginAck {
    static constexpr MsgType kType = MsgType::LoginAck;
    bool operator==(const LoginAck&) const = default;
};

struct NewOrder {
    static constexpr MsgType kType = MsgType::NewOrder;
    std::string account;
    ClientOrderId client_order_id = 0;
    Symbol symbol = Symbol::XAU;
    Side side = Side::Buy;
    OrderType order_type = OrderType::Limit;
    Qty qty = 0;
    Price price = 0;  // ignored for Market orders
    bool operator==(const NewOrder&) const = default;
};

struct Cancel {
    static constexpr MsgType kType = MsgType::Cancel;
    std::string account;
    ClientOrderId client_order_id = 0;
    Symbol symbol = Symbol::XAU;  // the gateway needs this to know which engine to route to!
    bool operator==(const Cancel&) const = default;
};

struct OrderAck {
    static constexpr MsgType kType = MsgType::OrderAck;
    std::string account;
    ClientOrderId client_order_id = 0;
    OrderId order_id = 0;
    bool operator==(const OrderAck&) const = default;
};

struct CancelAck {
    static constexpr MsgType kType = MsgType::CancelAck;
    std::string account;
    ClientOrderId client_order_id = 0;
    bool operator==(const CancelAck&) const = default;
};

struct Reject {
    static constexpr MsgType kType = MsgType::Reject;
    std::string account;
    ClientOrderId client_order_id = 0;
    RejectReason reason = RejectReason::Unknown;
    bool operator==(const Reject&) const = default;
};

struct Fill {
    static constexpr MsgType kType = MsgType::Fill;
    std::string account;
    ClientOrderId client_order_id = 0;
    OrderId order_id = 0;
    Symbol symbol = Symbol::XAU;
    Side side = Side::Buy;
    Price price = 0;
    Qty qty = 0;
    Qty remaining_qty = 0;  // 0 = this order is completely done
    bool operator==(const Fill&) const = default;
};

struct Subscribe {
    static constexpr MsgType kType = MsgType::Subscribe;
    bool operator==(const Subscribe&) const = default;
};

struct Quote {
    static constexpr MsgType kType = MsgType::Quote;
    Symbol symbol = Symbol::XAU;
    Price bid_price = 0;
    Qty bid_qty = 0;  // 0 = no bids
    Price ask_price = 0;
    Qty ask_qty = 0;  // 0 = no asks
    // TODO(STRETCH): send the top N levels (depth), not just the best one
    bool operator==(const Quote&) const = default;
};

struct Trade {
    static constexpr MsgType kType = MsgType::Trade;
    Symbol symbol = Symbol::XAU;
    Price price = 0;
    Qty qty = 0;
    Side aggressor_side = Side::Buy;  // side of the order that CROSSED the spread (the "taker")
    bool operator==(const Trade&) const = default;
};

struct Heartbeat {
    static constexpr MsgType kType = MsgType::Heartbeat;
    uint64_t last_seq = 0;
    bool operator==(const Heartbeat&) const = default;
};

struct FairValue {
    static constexpr MsgType kType = MsgType::FairValue;
    Price xau = 0;
    Price xag = 0;
    bool operator==(const FairValue&) const = default;
};

struct Ping {
    static constexpr MsgType kType = MsgType::Ping;
    bool operator==(const Ping&) const = default;
};

struct Pong {
    static constexpr MsgType kType = MsgType::Pong;
    bool operator==(const Pong&) const = default;
};

// A Message is exactly ONE of the above. std::variant is a type-safe union.
// Handle it with std::visit, or check with std::get_if<NewOrder>(&msg).
using Message = std::variant<Login, LoginAck, NewOrder, Cancel, OrderAck, CancelAck, Reject, Fill,
                             Subscribe, Quote, Trade, Heartbeat, FairValue, Ping, Pong>;

inline MsgType type_of(const Message& m) {
    return std::visit([](const auto& x) { return std::decay_t<decltype(x)>::kType; }, m);
}

// ------------------------------------------------------------- helpers
constexpr Side opposite(Side s) { return s == Side::Buy ? Side::Sell : Side::Buy; }

constexpr std::string_view to_string(Symbol s) { return s == Symbol::XAU ? "XAU" : "XAG"; }
constexpr std::string_view to_string(Side s) { return s == Side::Buy ? "BUY" : "SELL"; }
constexpr std::string_view to_string(OrderType t) { return t == OrderType::Limit ? "LIMIT" : "MARKET"; }

// "XAU" -> Symbol::XAU, anything else -> nullopt
inline std::optional<Symbol> parse_symbol(std::string_view s) {
    if (s == "XAU") return Symbol::XAU;
    if (s == "XAG") return Symbol::XAG;
    return std::nullopt;
}

// TODO(P1): write `std::string_view to_string(RejectReason)` with a switch. You'll want it in logs.

// Let log::info("side=", side) work.
inline std::ostream& operator<<(std::ostream& os, Symbol s) { return os << to_string(s); }
inline std::ostream& operator<<(std::ostream& os, Side s) { return os << to_string(s); }
inline std::ostream& operator<<(std::ostream& os, OrderType t) { return os << to_string(t); }

}  // namespace mms
