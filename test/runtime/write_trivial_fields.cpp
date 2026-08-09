#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

TEST(WriteTrivialFields, RoundTripsTrivialFieldsLittleEndian) {
  FIELD_LIST_SCHEMA =
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>
    >;

  test_field_list original{};
  original["a"_f] = 0xdeadbeef;
  original["b"_f] = 0xcafed00d;

  FIELD_LIST_LE_ROUNDTRIP_CHECK(original, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["a"_f], 0xdeadbeef);
    EXPECT_EQ((*result)["b"_f], 0xcafed00d);
  });
}

TEST(WriteTrivialFields, RoundTripsTrivialFieldsBigEndian) {
  FIELD_LIST_SCHEMA =
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>
    >;

  test_field_list original{};
  original["a"_f] = 0xdeadbeef;
  original["b"_f] = 0xcafed00d;

  FIELD_LIST_BE_ROUNDTRIP_CHECK(original, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["a"_f], 0xdeadbeef);
    EXPECT_EQ((*result)["b"_f], 0xcafed00d);
  });
}

// Round-trip alone would pass even if both directions ignored endianness, so
// pin the bytes that actually reach the stream.
TEST(WriteTrivialFields, EmitsDeclaredByteOrder) {
  using test_field_list =
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>
    >;

  test_field_list original{};
  original["a"_f] = 0xdeadbeef;

  std::stringstream le(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::stream_cast_le<test_field_list>(le, original).has_value());
  EXPECT_EQ(le.str(), std::string("\xef\xbe\xad\xde", 4));

  std::stringstream be(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::stream_cast_be<test_field_list>(be, original).has_value());
  EXPECT_EQ(be.str(), std::string("\xde\xad\xbe\xef", 4));
}

TEST(WriteTrivialFields, WritesFieldsInDeclarationOrder) {
  using test_field_list =
    s2s::struct_field_list<
      s2s::basic_field<"first", u8, s2s::field_size<s2s::fixed<1>>>,
      s2s::basic_field<"second", u8, s2s::field_size<s2s::fixed<1>>>,
      s2s::basic_field<"third", u8, s2s::field_size<s2s::fixed<1>>>
    >;

  test_field_list original{};
  original["first"_f] = 0x11;
  original["second"_f] = 0x22;
  original["third"_f] = 0x33;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::stream_cast_le<test_field_list>(stream, original).has_value());
  EXPECT_EQ(stream.str(), std::string("\x11\x22\x33", 3));
}
