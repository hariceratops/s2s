// The byte-order axis, write side — real-stream tier.
// Spec:   dev/specs/byte-order-is-data-not-a-template-parameter.md
// Design: dev/design/byte-order-is-data-not-a-template-parameter.md §10
//
// Separate from byte_order_axis_read.cpp rather than one round-trip: a
// round-trip stays green if both directions are wrong the same way.

#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"

using namespace s2s_literals;

namespace {
using order_marker =
  s2s::struct_field_list<
    s2s::fixed_array_field<"marker", u8, 2>
  >;

using order_guide =
  s2s::order_from<s2s::match_field<"marker">,
    s2s::order_switch<
      s2s::order_case<std::array<u8, 2>{'I', 'I'}, std::endian::little>,
      s2s::order_case<std::array<u8, 2>{'M', 'M'}, std::endian::big>>>;

using tiff_header =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", order_marker, order_guide>,
    s2s::magic_number<"magic", u16, 2_B, 42>,
    s2s::basic_field<"ifd_offset", u32>
  >;

// The marker is ordinary struct data and keeps its setter: assigning it is how
// a caller chooses the output order, which is why it is not a frozen field.
auto make_header(std::array<u8, 2> marker, u32 offset) -> tiff_header {
  tiff_header obj{};
  obj["byte_order"_f]["marker"_f] = marker;
  obj["ifd_offset"_f] = offset;
  return obj;
}

auto as_bytes(const std::string& s) -> std::vector<u8> {
  return std::vector<u8>(s.begin(), s.end());
}

using tiff_ifd =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", order_marker, order_guide>,
    s2s::basic_field<"entry_count", u16, 2_B>
  >;

using nested_once =
  s2s::struct_field_list<
    s2s::struct_field<"ifd", tiff_ifd>,
    s2s::basic_field<"next_ifd", u32>
  >;
} /* namespace */

TEST(ByteOrderAxisWrite, AnIIMarkerWritesEverythingAfterItLittleEndian) {
  FIELD_LIST_SCHEMA = tiff_header;

  FIELD_LIST_ROUNDTRIP_CHECK(make_header(std::array<u8, 2>{'I', 'I'}, 0xcafed00d), {
    ASSERT_TRUE(written.has_value());
    EXPECT_EQ(as_bytes(bytes),
              (std::vector<u8>{'I', 'I', 0x2a, 0x00, 0x0d, 0xd0, 0xfe, 0xca}));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["ifd_offset"_f], 0xcafed00du);
  });
}

TEST(ByteOrderAxisWrite, AnMMMarkerWritesEverythingAfterItBigEndian) {
  FIELD_LIST_SCHEMA = tiff_header;

  FIELD_LIST_ROUNDTRIP_CHECK(make_header(std::array<u8, 2>{'M', 'M'}, 0xcafed00d), {
    ASSERT_TRUE(written.has_value());
    EXPECT_EQ(as_bytes(bytes),
              (std::vector<u8>{'M', 'M', 0x00, 0x2a, 0xca, 0xfe, 0xd0, 0x0d}));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["ifd_offset"_f], 0xcafed00du);
  });
}

// The criterion that the marker keeps its setter, stated as the thing a caller
// would actually do: the same struct type and the same offset, one assignment
// apart, and the bytes come out in the other order.
TEST(ByteOrderAxisWrite, SettingTheMarkerChoosesTheOutputOrder) {
  FIELD_LIST_SCHEMA = tiff_header;

  std::stringstream little(std::ios::in | std::ios::out | std::ios::binary);
  std::stringstream big(std::ios::in | std::ios::out | std::ios::binary);

  auto le = s2s::stream_cast<test_field_list>(
    little, make_header(std::array<u8, 2>{'I', 'I'}, 0xcafed00d));
  auto be = s2s::stream_cast<test_field_list>(
    big, make_header(std::array<u8, 2>{'M', 'M'}, 0xcafed00d));

  ASSERT_TRUE(le.has_value());
  ASSERT_TRUE(be.has_value());
  EXPECT_NE(little.str(), big.str());
  EXPECT_EQ(as_bytes(little.str())[4], 0x0d);
  EXPECT_EQ(as_bytes(big.str())[4], 0xca);
}

// Resolution at depth is its own code path on write, so it gets its own case
// rather than riding on the read side's nesting tests.
TEST(ByteOrderAxisWrite, ANestedAnnouncementDecidesTheOrderOnWriteToo) {
  FIELD_LIST_SCHEMA = nested_once;

  test_field_list obj{};
  obj["ifd"_f]["byte_order"_f]["marker"_f] = std::array<u8, 2>{'M', 'M'};
  obj["ifd"_f]["entry_count"_f] = 300;
  obj["next_ifd"_f] = 0xcafed00d;

  FIELD_LIST_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    EXPECT_EQ(as_bytes(bytes),
              (std::vector<u8>{'M', 'M', 0x01, 0x2c, 0xca, 0xfe, 0xd0, 0x0d}));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["ifd"_f]["entry_count"_f], 300);
    EXPECT_EQ((*result)["next_ifd"_f], 0xcafed00du);
  });
}
