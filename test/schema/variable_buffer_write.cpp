#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

namespace {
using prefixed_schema =
  s2s::struct_field_list<
    s2s::basic_field<"str_len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
    s2s::str_field<"str", s2s::field_size<s2s::len_from_field<"str_len">>>,
    s2s::basic_field<"vec_len", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::vec_field<"vec", u16, s2s::field_size<s2s::len_from_field<"vec_len">>>
  >;

auto populated() -> prefixed_schema {
  prefixed_schema obj{};
  obj["str"_f] = "foo in bar";
  obj["vec"_f] = std::vector<u16>{0x1122, 0x3344, 0x5566};
  return obj;
}

auto expect_matches_populated(const prefixed_schema& actual) -> void {
  EXPECT_EQ(actual["str"_f], std::string("foo in bar"));
  EXPECT_EQ(actual["vec"_f], (std::vector<u16>{0x1122, 0x3344, 0x5566}));
}
} /* namespace */

TEST(VariableBufferWrite, RoundTripsLengthPrefixedFieldsLittleEndian) {
  using test_field_list = prefixed_schema;

  FIELD_LIST_LE_ROUNDTRIP_CHECK(populated(), {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    expect_matches_populated(*result);
  });
}

TEST(VariableBufferWrite, RoundTripsLengthPrefixedFieldsBigEndian) {
  using test_field_list = prefixed_schema;

  FIELD_LIST_BE_ROUNDTRIP_CHECK(populated(), {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    expect_matches_populated(*result);
  });
}

// The length slot is derived, not stored: whatever the caller left in it is
// overwritten by the container's real size.
TEST(VariableBufferWrite, IgnoresTheStoredLengthAndDerivesIt) {
  using test_field_list =
    s2s::struct_field_list<
      s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::vec_field<"vec", u16, s2s::field_size<s2s::len_from_field<"len">>>
    >;

  test_field_list obj{};
  obj["vec"_f] = std::vector<u16>{0xaabb, 0xccdd};

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::stream_cast_le<test_field_list>(stream, obj).has_value());
  EXPECT_EQ(stream.str(), std::string("\x02\x00\x00\x00\xbb\xaa\xdd\xcc", 8));
}

// A derived length is emitted in its own declared width and byte order, not
// the container's.
TEST(VariableBufferWrite, EmitsDerivedLengthInDeclaredWidthAndByteOrder) {
  using test_field_list =
    s2s::struct_field_list<
      s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::vec_field<"vec", u16, s2s::field_size<s2s::len_from_field<"len">>>
    >;

  test_field_list obj{};
  obj["vec"_f] = std::vector<u16>{0xaabb, 0xccdd};

  std::stringstream be(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::stream_cast_be<test_field_list>(be, obj).has_value());
  EXPECT_EQ(be.str(), std::string("\x00\x00\x00\x02\xaa\xbb\xcc\xdd", 8));
}

TEST(VariableBufferWrite, RoundTripsEmptyContainers) {
  using test_field_list = prefixed_schema;

  prefixed_schema obj{};

  FIELD_LIST_LE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE((*result)["str"_f].empty());
    EXPECT_TRUE((*result)["vec"_f].empty());
  });
}

// Silent truncation would produce a stream that reads back as a shorter
// container, so a length that does not fit its slot must fail the write.
TEST(VariableBufferWrite, RejectsLengthTooWideForItsDeclaredSlot) {
  using test_field_list =
    s2s::struct_field_list<
      s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<1>>>,
      s2s::vec_field<"vec", u8, s2s::field_size<s2s::len_from_field<"len">>>
    >;

  test_field_list obj{};
  obj["vec"_f] = std::vector<u8>(300, 0x5a);

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::stream_cast_le<test_field_list>(stream, obj);
  ASSERT_FALSE(written.has_value());
  EXPECT_EQ(written.error().failure_reason, s2s::error_reason::validation_failure);
  EXPECT_EQ(written.error().failed_at, "len");
  EXPECT_TRUE(stream.str().empty());
}

TEST(VariableBufferWrite, AcceptsALengthThatExactlyFillsItsSlot) {
  using test_field_list =
    s2s::struct_field_list<
      s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<1>>>,
      s2s::vec_field<"vec", u8, s2s::field_size<s2s::len_from_field<"len">>>
    >;

  test_field_list obj{};
  obj["vec"_f] = std::vector<u8>(255, 0x5a);

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::stream_cast_le<test_field_list>(stream, obj).has_value());
  EXPECT_EQ(stream.str().size(), 256u);
  EXPECT_EQ(static_cast<u8>(stream.str()[0]), 255);
}
