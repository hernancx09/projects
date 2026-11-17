#pragma once
// =============================================================================
// Wire protocol: how Messages become bytes and back.   <-- START HERE
// =============================================================================
//
// PART 1: TCP FRAMING
// -------------------
// TCP gives you a reliable, ordered STREAM OF BYTES, not a stream of
// messages. If the sender writes two 40-byte messages, one recv() on the
// other end might return:
//     - all 80 bytes,
//     - 13 bytes (part of the first message),
//     - 40 + 7 bytes (the first message plus the start of the second), ...
// That depends on buffering, Nagle's algorithm, MTU, and timing. So we mark
// boundaries ourselves with LENGTH-PREFIXED FRAMING:
//
//     +-----------+-----------+------------------------------+
//     | length    | type      | body (fields, big-endian)    |
//     | u16 BE    | u8        |                              |
//     +-----------+-----------+------------------------------+
//                 |<------------- `length` bytes ------------>|
//
// The receiver buffers bytes until it has 2, reads `length`, then waits
// until it has `length` more. That's one complete message. Repeat.
// (FrameBuffer below does this.)
//
// BIG-ENDIAN ("network byte order"): the most significant byte goes first.
//     uint16 0x1234 -> bytes 0x12 0x34      (x86 memory holds 0x34 0x12)
//
// STRINGS: account names are FIXED-WIDTH (16 bytes, zero padded). Fixed
// widths make every message of a given type the same size, which is easy and
// fast to parse. Real exchange protocols (ITCH, OUCH) do the same.
//
// PART 2: UDP DATAGRAMS (market data)
// -----------------------------------
// UDP keeps message boundaries (one sendto() = one recvfrom()), but packets
// can be LOST, DUPLICATED, or REORDERED, and nothing tells you. So each
// datagram gets a header in front of a normal frame:
//
//     +-----------+-------------+---------+------------------------+
//     | seq u64   | send_ns u64 | ver u8  | one frame (as above)   |
//     +-----------+-------------+---------+------------------------+
//
//   seq      +1 per packet per publisher. Got 41 then 43? You lost 42.
//   send_ns  publisher's clock, for measuring one-way latency. (Containers
//            on one host share a clock, so this works here. Real machines
//            need clock sync, e.g. PTP.)
//   ver      protocol version, so the format can change later.
//
// Stay under kMaxDatagram. Ethernet MTU is 1500 bytes. Larger datagrams get
// split into IP fragments, and losing ANY fragment loses the whole datagram.
// =============================================================================
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "common/messages.hpp"

namespace mms::protocol {

constexpr std::size_t kLengthFieldSize = 2;
constexpr std::size_t kMaxFrameBody = 1024;  // reject bigger: protects against garbage / malicious lengths
constexpr std::size_t kAccountWidth = 16;

constexpr uint8_t kUdpVersion = 1;
constexpr std::size_t kUdpHeaderSize = 8 + 8 + 1;
constexpr std::size_t kMaxDatagram = 1200;

// ----------------------------------------------------------- byte helpers
// Appends big-endian values to a byte vector.
class ByteWriter {
public:
    explicit ByteWriter(std::vector<uint8_t>& out) : out_(out) {}
    void u8(uint8_t v);
    void u16(uint16_t v);
    void u32(uint32_t v);
    void u64(uint64_t v);
    void i32(int32_t v) { u32(static_cast<uint32_t>(v)); }  // two's complement: same bits, just reinterpreted
    void i64(int64_t v) { u64(static_cast<uint64_t>(v)); }
    void fixed_str(std::string_view s, std::size_t width);  // zero-padded; error if s is too long

private:
    std::vector<uint8_t>& out_;
};

// Reads big-endian values from a byte span. If you read past the end,
// ok() turns false and reads return 0. Check ok() ONCE after reading a whole
// message instead of after every field.
class ByteReader {
public:
    explicit ByteReader(std::span<const uint8_t> data) : data_(data) {}
    uint8_t u8();
    uint16_t u16();
    uint32_t u32();
    uint64_t u64();
    int32_t i32() { return static_cast<int32_t>(u32()); }
    int64_t i64() { return static_cast<int64_t>(u64()); }
    std::string fixed_str(std::size_t width);  // strips the zero padding
    bool ok() const { return ok_; }
    std::size_t remaining() const { return ok_ ? data_.size() - pos_ : 0; }

private:
    bool take(std::size_t n);  // advance by n bytes, or set ok_=false
    std::span<const uint8_t> data_;
    std::size_t pos_ = 0;
    bool ok_ = true;
};

// ------------------------------------------------------------ TCP frames
// Appends one complete frame (length + type + body) to `out`.
void encode_frame(const Message& msg, std::vector<uint8_t>& out);

// Decodes `type byte + body` (i.e. everything AFTER the length field).
// Returns nullopt on unknown type, truncated body, or trailing junk.
std::optional<Message> decode_body(std::span<const uint8_t> type_and_body);

// Reassembles frames from arbitrary chunks of a TCP stream.
//
//   FrameBuffer fb;
//   fb.append(bytes_from_recv, n);
//   Message m;
//   while (fb.next(m) == FrameBuffer::Status::Ok) { handle(m); }
//   if status == Error -> the peer sent garbage. Close the connection.
class FrameBuffer {
public:
    enum class Status { Ok, NeedMore, Error };

    void append(const uint8_t* data, std::size_t n);
    Status next(Message& out);
    std::size_t buffered() const { return buf_.size() - read_pos_; }

private:
    std::vector<uint8_t> buf_;
    std::size_t read_pos_ = 0;  // bytes before this are already consumed
};

// ---------------------------------------------------------- UDP datagrams
struct Datagram {
    uint64_t seq = 0;
    uint64_t send_ns = 0;
    Message msg;
};

void encode_datagram(uint64_t seq, uint64_t send_ns, const Message& msg, std::vector<uint8_t>& out);
std::optional<Datagram> decode_datagram(std::span<const uint8_t> data);

// Wall-clock nanoseconds since the Unix epoch (for send_ns / latency).
uint64_t now_ns();

}  // namespace mms::protocol
