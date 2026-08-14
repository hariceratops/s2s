#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

namespace {
using magic_schema =
  s2s::struct_field_list<
    s2s::magic_string<"magic_str", "GIF">,
    s2s::magic_number<"magic_num", u32, 4_B, 0xdeadbeef>,
    s2s::magic_byte_array<"magic_arr", 4, std::array<unsigned char, 4>{0xde, 0xad, 0xbe, 0xef}>,
    s2s::basic_field<"payload", u32, 4_B>
  >;

auto populated() -> magic_schema {
  magic_schema obj{};
  obj["magic_str"_f] = s2s::fixed_string<3>("GIF");
  obj["magic_num"_f] = 0xdeadbeef;
  obj["magic_arr"_f] = std::array<unsigned char, 4>{0xde, 0xad, 0xbe, 0xef};
  obj["payload"_f] = 0xcafed00d;
  return obj;
}

auto expect_matches_populated(const magic_schema& actual) -> void {
  EXPECT_EQ(std::string_view(actual["magic_str"_f].data()), std::string_view("GIF"));
  EXPECT_EQ(actual["magic_num"_f], 0xdeadbeef);
  EXPECT_EQ(actual["magic_arr"_f], (std::array<unsigned char, 4>{0xde, 0xad, 0xbe, 0xef}));
  EXPECT_EQ(actual["payload"_f], 0xcafed00d);
}
} /* namespace */

TEST(MagicWrite, RoundTripsMagicFieldsLittleEndian) {
  using test_field_list = magic_schema;

  FIELD_LIST_LE_ROUNDTRIP_CHECK(populated(), {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    expect_matches_populated(*result);
  });
}

TEST(MagicWrite, RoundTripsMagicFieldsBigEndian) {
  using test_field_list = magic_schema;

  FIELD_LIST_BE_ROUNDTRIP_CHECK(populated(), {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    expect_matches_populated(*result);
  });
}

TEST(MagicWrite, RejectsWrongMagicString) {
  using test_field_list = magic_schema;

  auto obj = populated();
  obj["magic_str"_f] = s2s::fixed_string<3>("NAH");

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::stream_cast_le<test_field_list>(stream, obj);
  ASSERT_FALSE(written.has_value());
  EXPECT_EQ(written.error().failure_reason, s2s::error_reason::validation_failure);
  EXPECT_EQ(written.error().failed_at, "magic_str");
}

TEST(MagicWrite, RejectsWrongMagicNumberInBothByteOrders) {
  using test_field_list = magic_schema;

  auto obj = populated();
  obj["magic_num"_f] = 0xbeefbeef;

  std::stringstream le(std::ios::in | std::ios::out | std::ios::binary);
  auto le_written = s2s::stream_cast_le<test_field_list>(le, obj);
  ASSERT_FALSE(le_written.has_value());
  EXPECT_EQ(le_written.error().failure_reason, s2s::error_reason::validation_failure);
  EXPECT_EQ(le_written.error().failed_at, "magic_num");

  std::stringstream be(std::ios::in | std::ios::out | std::ios::binary);
  auto be_written = s2s::stream_cast_be<test_field_list>(be, obj);
  ASSERT_FALSE(be_written.has_value());
  EXPECT_EQ(be_written.error().failure_reason, s2s::error_reason::validation_failure);
  EXPECT_EQ(be_written.error().failed_at, "magic_num");
}

TEST(MagicWrite, RejectsWrongMagicByteArray) {
  using test_field_list = magic_schema;

  auto obj = populated();
  obj["magic_arr"_f] = std::array<unsigned char, 4>{0x00, 0x00, 0x00, 0x00};

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::stream_cast_le<test_field_list>(stream, obj);
  ASSERT_FALSE(written.has_value());
  EXPECT_EQ(written.error().failed_at, "magic_arr");
}

// Fail-fast, not rollback: fields already written stay on the stream, but the
// offending field must contribute nothing.
TEST(MagicWrite, OffendingFieldContributesNoBytes) {
  using test_field_list =
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, 4_B>,
      s2s::magic_number<"magic_num", u32, 4_B, 0xdeadbeef>,
      s2s::basic_field<"b", u32, 4_B>
    >;

  test_field_list obj{};
  obj["a"_f] = 0x11223344;
  obj["magic_num"_f] = 0xbeefbeef;
  obj["b"_f] = 0x55667788;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_FALSE(s2s::stream_cast_le<test_field_list>(stream, obj).has_value());
  EXPECT_EQ(stream.str(), std::string("\x44\x33\x22\x11", 4));
}
