// =============================================================================
// Protocol tests. These FAIL until you implement common/protocol.cpp. Make them pass.
//
//   cmake --build build && ctest --test-dir build --output-on-failure
//   or one test:  ./build/bin/unit_tests --gtest_filter='Protocol.*'
//
// The most important test here is PartialDelivery: it simulates TCP handing
// you bytes in arbitrary chunks, which is the whole reason framing exists.
// TODO(P1): add your own tests: every message type round-trips; an
//   unknown type byte is rejected; a truncated body is rejected.
// =============================================================================
#include <gtest/gtest.h>

#include "common/protocol.hpp"

using namespace mms;
using namespace mms::protocol;

namespace {

NewOrder sample_order() {
    NewOrder o;
    o.account = "mm_1";
    o.client_order_id = 42;
    o.symbol = Symbol::XAG;
    o.side = Side::Sell;
    o.order_type = OrderType::Limit;
    o.qty = 7;
    o.price = 3015;  // $30.15
    return o;
}

}  // namespace

TEST(Protocol, WriterIsBigEndian) {
    std::vector<uint8_t> out;
    ByteWriter w(out);
    w.u32(0x01020304);
    ASSERT_EQ(out.size(), 4u);
    EXPECT_EQ(out[0], 0x01);  // most significant byte FIRST
    EXPECT_EQ(out[3], 0x04);
}

TEST(Protocol, ReaderPastEndSetsNotOk) {
    std::vector<uint8_t> data{0x00, 0x01};
    ByteReader r(data);
    EXPECT_EQ(r.u16(), 1);
    EXPECT_TRUE(r.ok());
    r.u8();  // nothing left
    EXPECT_FALSE(r.ok());
}

TEST(Protocol, RoundTripNewOrder) {
    std::vector<uint8_t> bytes;
    encode_frame(sample_order(), bytes);

    FrameBuffer fb;
    fb.append(bytes.data(), bytes.size());
    Message m;
    ASSERT_EQ(fb.next(m), FrameBuffer::Status::Ok);
    ASSERT_TRUE(std::holds_alternative<NewOrder>(m));
    EXPECT_EQ(std::get<NewOrder>(m), sample_order());
    EXPECT_EQ(fb.next(m), FrameBuffer::Status::NeedMore);  // nothing left
}

TEST(Protocol, LengthPrefixMatchesBody) {
    std::vector<uint8_t> bytes;
    encode_frame(Login{"bob"}, bytes);
    // [len_hi][len_lo][type][16 bytes account]
    ASSERT_EQ(bytes.size(), 2u + 1u + kAccountWidth);
    const unsigned len = (bytes[0] << 8) | bytes[1];
    EXPECT_EQ(len, 1u + kAccountWidth);
    EXPECT_EQ(bytes[2], static_cast<uint8_t>(MsgType::Login));
}

TEST(Protocol, PartialDelivery) {
    // TCP may hand us ONE BYTE at a time. The FrameBuffer must wait patiently.
    std::vector<uint8_t> bytes;
    encode_frame(sample_order(), bytes);

    FrameBuffer fb;
    Message m;
    for (std::size_t i = 0; i + 1 < bytes.size(); ++i) {
        fb.append(&bytes[i], 1);
        ASSERT_EQ(fb.next(m), FrameBuffer::Status::NeedMore) << "after byte " << i;
    }
    fb.append(&bytes.back(), 1);
    ASSERT_EQ(fb.next(m), FrameBuffer::Status::Ok);
    EXPECT_EQ(std::get<NewOrder>(m), sample_order());
}

TEST(Protocol, TwoFramesInOneChunk) {
    // ...or TWO messages glued together in one recv().
    std::vector<uint8_t> bytes;
    encode_frame(Ping{}, bytes);
    encode_frame(sample_order(), bytes);

    FrameBuffer fb;
    fb.append(bytes.data(), bytes.size());
    Message m;
    ASSERT_EQ(fb.next(m), FrameBuffer::Status::Ok);
    EXPECT_TRUE(std::holds_alternative<Ping>(m));
    ASSERT_EQ(fb.next(m), FrameBuffer::Status::Ok);
    EXPECT_TRUE(std::holds_alternative<NewOrder>(m));
}

TEST(Protocol, RejectsHugeLength) {
    // A garbage length field must be an Error, not "wait for 65535 bytes".
    std::vector<uint8_t> bytes{0xFF, 0xFF, 0x01};
    FrameBuffer fb;
    fb.append(bytes.data(), bytes.size());
    Message m;
    EXPECT_EQ(fb.next(m), FrameBuffer::Status::Error);
}

TEST(Protocol, AccountTooLongThrows) {
    std::vector<uint8_t> bytes;
    EXPECT_ANY_THROW(encode_frame(Login{"this_name_is_way_too_long_for_16"}, bytes));
}

TEST(Protocol, DatagramRoundTrip) {
    std::vector<uint8_t> bytes;
    Quote q{Symbol::XAU, 240000, 5, 240010, 3};
    encode_datagram(17, 123456789, q, bytes);

    auto dg = decode_datagram(bytes);
    ASSERT_TRUE(dg.has_value());
    EXPECT_EQ(dg->seq, 17u);
    EXPECT_EQ(dg->send_ns, 123456789u);
    EXPECT_EQ(std::get<Quote>(dg->msg), q);
}

TEST(Protocol, DatagramTruncatedIsRejected) {
    std::vector<uint8_t> bytes;
    encode_datagram(1, 1, Heartbeat{1}, bytes);
    bytes.pop_back();
    EXPECT_FALSE(decode_datagram(bytes).has_value());
}
