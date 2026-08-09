#include <gtest/gtest.h>
#include <cstdio>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

namespace {
using aggregate_schema =
  s2s::struct_field_list<
    s2s::fixed_array_field<"arr", u16, 3>,
    s2s::fixed_string_field<"name", 4>,
    s2s::c_arr_field<"carr", u32, 2>,
    s2s::c_str_field<"cstr", 3>
  >;

auto populated() -> aggregate_schema {
  aggregate_schema obj{};
  obj["arr"_f] = std::array<u16, 3>{0x1122, 0x3344, 0x5566};
  obj["name"_f] = s2s::fixed_string<4>("abcd");
  obj["carr"_f][0] = 0xdeadbeef;
  obj["carr"_f][1] = 0xcafed00d;
  std::snprintf(obj["cstr"_f], 4, "xyz");
  return obj;
}

auto expect_matches_populated(const aggregate_schema& actual) -> void {
  const auto expected = populated();
  EXPECT_EQ(actual["arr"_f], expected["arr"_f]);
  EXPECT_EQ(actual["name"_f], expected["name"_f]);
  EXPECT_EQ(actual["carr"_f][0], 0xdeadbeef);
  EXPECT_EQ(actual["carr"_f][1], 0xcafed00d);
  EXPECT_EQ(std::string_view(actual["cstr"_f]), std::string_view("xyz"));
}
} /* namespace */

TEST(WriteFixedBufferFields, RoundTripsFixedSizeAggregatesLittleEndian) {
  using test_field_list = aggregate_schema;
  const auto original = populated();

  FIELD_LIST_LE_ROUNDTRIP_CHECK(original, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    expect_matches_populated(*result);
  });
}

TEST(WriteFixedBufferFields, RoundTripsFixedSizeAggregatesBigEndian) {
  using test_field_list = aggregate_schema;
  const auto original = populated();

  FIELD_LIST_BE_ROUNDTRIP_CHECK(original, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    expect_matches_populated(*result);
  });
}

// Each element is swapped individually — a whole-buffer reversal would give
// 66 55 44 33 22 11 and still round-trip, so pin the bytes.
TEST(WriteFixedBufferFields, ByteswapsPerElementNotPerBuffer) {
  using test_field_list = s2s::struct_field_list<s2s::fixed_array_field<"arr", u16, 3>>;

  test_field_list original{};
  original["arr"_f] = std::array<u16, 3>{0x1122, 0x3344, 0x5566};

  std::stringstream le(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::struct_write_le<test_field_list>(le, original).has_value());
  EXPECT_EQ(le.str(), std::string("\x22\x11\x44\x33\x66\x55", 6));

  std::stringstream be(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::struct_write_be<test_field_list>(be, original).has_value());
  EXPECT_EQ(be.str(), std::string("\x11\x22\x33\x44\x55\x66", 6));
}

// The swap has to descend into the inner array rather than treating each row
// as one opaque element.
TEST(WriteFixedBufferFields, ByteswapsNestedAggregatesElementWise) {
  using test_field_list =
    s2s::struct_field_list<s2s::fixed_array_field<"arr", std::array<u16, 2>, 2>>;

  test_field_list original{};
  original["arr"_f] = std::array<std::array<u16, 2>, 2>{{{0x1122, 0x3344}, {0x5566, 0x7788}}};

  std::stringstream be(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::struct_write_be<test_field_list>(be, original).has_value());
  EXPECT_EQ(be.str(), std::string("\x11\x22\x33\x44\x55\x66\x77\x88", 8));

  FIELD_LIST_BE_ROUNDTRIP_CHECK(original, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["arr"_f], original["arr"_f]);
  });
}

// Char elements have no byte order, so both directions must emit them
// unchanged rather than reversing the string.
TEST(WriteFixedBufferFields, LeavesCharBuffersUnswapped) {
  using test_field_list = s2s::struct_field_list<s2s::fixed_string_field<"name", 4>>;

  test_field_list original{};
  original["name"_f] = s2s::fixed_string<4>("abcd");

  std::stringstream be(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::struct_write_be<test_field_list>(be, original).has_value());
  EXPECT_EQ(be.str().substr(0, 4), std::string("abcd"));
}
