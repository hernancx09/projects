// Implementation of the wire protocol. Read protocol.hpp first.
// Tests for this file: tests/test_protocol.cpp. Make them pass!
#include "common/protocol.hpp"

#include <chrono>
#include <stdexcept>

#include "common/todo.hpp"

namespace mms::protocol {

// =============================================================== ByteWriter
void ByteWriter::u8(uint8_t v) { out_.push_back(v); }

// Worked example: big-endian = most significant byte FIRST.
//   v = 0x1234 -> (v >> 8) & 0xFF = 0x12 goes first, then v & 0xFF = 0x34.
// Shifting works the same on every CPU, so this code is portable.
void ByteWriter::u16(uint16_t v) {
    out_.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out_.push_back(static_cast<uint8_t>(v & 0xFF));
}

void ByteWriter::u32(uint32_t v) {
    // TODO(P1): same idea as u16, with 4 bytes: shifts 24, 16, 8, 0
    not_implemented();
}

void ByteWriter::u64(uint64_t v) {
    // TODO(P1): 8 bytes: shifts 56, 48, ..., 0 (a loop is fine)
    not_implemented();
}

void ByteWriter::fixed_str(std::string_view s, std::size_t width) {
    // TODO(P1): if s.size() > width -> throw std::invalid_argument
    //   append the bytes of s, then (width - s.size()) zero bytes
    not_implemented();
}

// =============================================================== ByteReader
bool ByteReader::take(std::size_t n) {
    // TODO(P1): if !ok_ or pos_ + n > data_.size(): ok_ = false; return false
    //   (no advance here. The caller reads data_[pos_..pos_+n) then advances pos_ += n)
    //   Alternatively: return true and let the caller advance. Pick one style.
    not_implemented();
}

uint8_t ByteReader::u8() {
    // TODO(P1): if (!take(1)) return 0; return data_[pos_++];
    not_implemented();
}

uint16_t ByteReader::u16() {
    // TODO(P1): the reverse of ByteWriter::u16:
    //   uint16_t v = (uint16_t(data_[pos_]) << 8) | data_[pos_ + 1];
    // Careful: data_[i] is uint8_t, and uint8_t << 8 gets promoted to int, which
    //   is fine for 16 and 32 bits. For u64, CAST TO uint64_t BEFORE SHIFTING,
    //   or << 56 on an int is undefined behavior. (UBSan will catch it: -DMMS_SANITIZERS=ON)
    not_implemented();
}

uint32_t ByteReader::u32() {
    // TODO(P1)
    not_implemented();
}

uint64_t ByteReader::u64() {
    // TODO(P1)
    not_implemented();
}

std::string ByteReader::fixed_str(std::size_t width) {
    // TODO(P1): take(width) bytes, then cut at the first '\0'
    not_implemented();
}

// ==================================================== per-message encoding
// One overload per message type. encode_frame calls the right one via std::visit.
// THE ORDER OF FIELDS HERE *IS* THE PROTOCOL. Writer and reader must match exactly.
namespace {

// Worked example for one message:
void write_body(ByteWriter& w, const Login& m) { w.fixed_str(m.account, kAccountWidth); }

// Messages with no fields have an empty body (just the type byte).
void write_body(ByteWriter&, const LoginAck&) {}
void write_body(ByteWriter&, const Subscribe&) {}
void write_body(ByteWriter&, const Ping&) {}
void write_body(ByteWriter&, const Pong&) {}

void write_body(ByteWriter& w, const NewOrder& m) {
    // TODO(P1): account(16) client_order_id(u64) symbol(u8) side(u8) order_type(u8) qty(i32) price(i64)
    //   Enums: w.u8(static_cast<uint8_t>(m.side))
    not_implemented();
}

void write_body(ByteWriter& w, const Cancel& m) { not_implemented(); }     // TODO(P1)
void write_body(ByteWriter& w, const OrderAck& m) { not_implemented(); }   // TODO(P1)
void write_body(ByteWriter& w, const CancelAck& m) { not_implemented(); }  // TODO(P1)
void write_body(ByteWriter& w, const Reject& m) { not_implemented(); }     // TODO(P1)
void write_body(ByteWriter& w, const Fill& m) { not_implemented(); }       // TODO(P3)
void write_body(ByteWriter& w, const Quote& m) { not_implemented(); }      // TODO(P4)
void write_body(ByteWriter& w, const Trade& m) { not_implemented(); }      // TODO(P4)
void write_body(ByteWriter& w, const Heartbeat& m) { not_implemented(); }  // TODO(P4)
void write_body(ByteWriter& w, const FairValue& m) { not_implemented(); }  // TODO(P5)

}  // namespace

void encode_frame(const Message& msg, std::vector<uint8_t>& out) {
    // TODO(P1):
    //   1. Remember where this frame starts: auto start = out.size();
    //   2. Write a PLACEHOLDER length: w.u16(0)   (we don't know the body size yet)
    //   3. w.u8(static_cast<uint8_t>(type_of(msg)))
    //   4. std::visit([&](const auto& m) { write_body(w, m); }, msg);
    //   5. Compute len = out.size() - start - kLengthFieldSize.
    //      If len > kMaxFrameBody, throw.
    //   6. BACKPATCH: overwrite out[start] and out[start+1] with len, big-endian.
    // This "reserve then backpatch" trick avoids building the body in a temp buffer.
    not_implemented();
}

std::optional<Message> decode_body(std::span<const uint8_t> type_and_body) {
    // TODO(P1):
    //   ByteReader r(type_and_body);
    //   auto type = static_cast<MsgType>(r.u8());
    //   switch (type) {
    //     case MsgType::Login: { Login m; m.account = r.fixed_str(kAccountWidth); result = m; break; }
    //     case MsgType::NewOrder: { ... read fields in EXACTLY the order write_body wrote them ... }
    //     ...
    //     default: return std::nullopt;   // unknown type
    //   }
    //   if (!r.ok() || r.remaining() != 0) return std::nullopt;   // truncated, or extra junk
    //
    // VALIDATE ENUMS: a byte of 7 cast to Side is a Side that's neither Buy
    //   nor Sell. Check the range after reading. Never trust the network.
    not_implemented();
}

// ============================================================== FrameBuffer
void FrameBuffer::append(const uint8_t* data, std::size_t n) {
    // TODO(P1): buf_.insert(buf_.end(), data, data + n);
    //
    // Memory: consumed bytes (before read_pos_) pile up forever. When
    //   read_pos_ gets large (or == buf_.size()), erase the consumed prefix
    //   ("compaction"). TODO(STRETCH): a fixed-size ring buffer avoids the copy.
    not_implemented();
}

FrameBuffer::Status FrameBuffer::next(Message& out) {
    // TODO(P1):
    //   if buffered() < kLengthFieldSize -> NeedMore
    //   len = big-endian u16 at buf_[read_pos_]
    //   if len == 0 or len > kMaxFrameBody -> Error   (a peer sending garbage)
    //   if buffered() < kLengthFieldSize + len -> NeedMore   (DON'T consume anything yet!)
    //   body = std::span(buf_).subspan(read_pos_ + kLengthFieldSize, len)
    //   auto msg = decode_body(body); if (!msg) -> Error
    //   read_pos_ += kLengthFieldSize + len; out = std::move(*msg); return Ok
    not_implemented();
}

// ================================================================ datagrams
void encode_datagram(uint64_t seq, uint64_t send_ns, const Message& msg, std::vector<uint8_t>& out) {
    // TODO(P4): ByteWriter: u64(seq), u64(send_ns), u8(kUdpVersion), then encode_frame(msg, out).
    //   Throw if out.size() > kMaxDatagram.
    // TODO(STRETCH): put SEVERAL frames in one datagram (batching). Real feeds
    //   do this to cut per-packet overhead. The header would need a message count.
    not_implemented();
}

std::optional<Datagram> decode_datagram(std::span<const uint8_t> data) {
    // TODO(P4): read the header with ByteReader, check the version, then the
    //   rest is exactly one frame: check its length field matches, then decode_body.
    not_implemented();
}

uint64_t now_ns() {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count());
}

}  // namespace mms::protocol
