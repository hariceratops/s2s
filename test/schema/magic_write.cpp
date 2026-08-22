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

// The three magic fields are never assigned — they have no setter. Their
// values are on their constraints, and the write path takes them from there.
auto populated() -> magic_schema {
  magic_schema obj{};
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

// The bytes, not just the round trip: a struct nobody touched must still put
// the declared magic on the wire, or the round trip above would pass on a
// stream that agrees with itself and with nothing else.
TEST(MagicWrite, WritesTheDeclaredBytesFromAnUntouchedStruct) {
  using test_field_list = magic_schema;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::stream_cast_be<test_field_list>(stream, magic_schema{}).has_value());
  EXPECT_EQ(stream.str(),
            std::string("GIF\0"
                        "\xde\xad\xbe\xef"
                        "\xde\xad\xbe\xef"
                        "\0\0\0\0", 16));
}

// Fail-fast, not rollback: fields already written stay on the stream, but the
// offending field must contribute nothing. A magic field can no longer be the
// one that offends, so the trigger is an ordinary constraint the caller can
// still violate.
TEST(MagicWrite, OffendingFieldContributesNoBytes) {
  using test_field_list =
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, 4_B>,
      s2s::basic_field<"small", u32, 4_B, s2s::lt{10u}>,
      s2s::basic_field<"b", u32, 4_B>
    >;

  test_field_list obj{};
  obj["a"_f] = 0x11223344;
  obj["small"_f] = 0xbeefbeef;
  obj["b"_f] = 0x55667788;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_FALSE(s2s::stream_cast_le<test_field_list>(stream, obj).has_value());
  EXPECT_EQ(stream.str(), std::string("\x44\x33\x22\x11", 4));
}
