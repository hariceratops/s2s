#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"

using namespace s2s_literals;

TEST(S2STest, MagicNumber) {
  PREPARE_INPUT_FILE({
    u32 a = 0xdeadbeef;
    u32 b = 0xcafed00d;
    u32 x = 0xbeefbeef;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.write(reinterpret_cast<const char*>(&x), sizeof(x));
  });

  FIELD_LIST_SCHEMA =
    s2s::struct_field_list<
      s2s::magic_number<"magic_num", u32, s2s::field_size<s2s::fixed<4>>, 0xdeadbeef>,
      s2s::basic_field<"p", u32, s2s::field_size<s2s::fixed<4>>>
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    ASSERT_EQ(fields["magic_num"_f], 0xdeadbeef);
    ASSERT_EQ(fields["p"_f], 0xcafed00d);
  });
}

TEST(S2STest, MagicArray) {
  PREPARE_INPUT_FILE({
    u32 bytes_0_3 = 0xffffffff;
    u32 bytes_4_7 = 0xffffffff;
    u16 bytes_8_9 = 0xffff;
    u32 x = 0xcafed00d;
    file.write(reinterpret_cast<const char*>(&bytes_0_3), sizeof(bytes_0_3));
    file.write(reinterpret_cast<const char*>(&bytes_4_7), sizeof(bytes_4_7));
    file.write(reinterpret_cast<const char*>(&bytes_8_9), sizeof(bytes_8_9));
    file.write(reinterpret_cast<const char*>(&x), sizeof(x));
  });

  FIELD_LIST_SCHEMA =
    s2s::struct_field_list<
      s2s::magic_byte_array<"magic_arr", 10, std::array<unsigned char, 10>{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}>,
      s2s::basic_field<"size", u32, s2s::field_size<s2s::fixed<4>>>
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    ASSERT_EQ(fields["magic_arr"_f], (std::array<u8, 10>{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}));
    ASSERT_EQ(fields["size"_f], 0xcafed00d);
  });
}

TEST(S2STest, MagicString) {
  PREPARE_INPUT_FILE({
    const u8 str[] = "GIF";
    const auto str_len = 3;
    u32 x = 0xcafed00d;
    file.write(reinterpret_cast<const char*>(&str), str_len + 1);
    file.write(reinterpret_cast<const char*>(&x), sizeof(x));
  });

  FIELD_LIST_SCHEMA =
    s2s::struct_field_list<
      s2s::magic_string<"magic_str", "GIF">,
      s2s::basic_field<"size", u32, s2s::field_size<s2s::fixed<4>>>
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    ASSERT_EQ(std::string_view{fields["magic_str"_f].data()}, std::string_view{s2s::fixed_string("GIF").data()});
    ASSERT_EQ(fields["size"_f], 0xcafed00d);
  });
}

TEST(S2STest, FailingMagicStringRead) {
  PREPARE_INPUT_FILE({
    const u8 str[] = "NAH";
    const auto str_len = 3;
    u32 x = 0xcafed00d;
    file.write(reinterpret_cast<const char*>(&str), str_len + 1);
    file.write(reinterpret_cast<const char*>(&x), sizeof(x));
  });

  FIELD_LIST_SCHEMA =
    s2s::struct_field_list<
      s2s::magic_string<"magic_str", "GIF">,
      s2s::basic_field<"size", u32, s2s::field_size<s2s::fixed<4>>>
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_FALSE(result.has_value());
    auto err = result.error();
    ASSERT_EQ(err.failure_reason, s2s::error_reason::validation_failure);
    ASSERT_EQ(err.failed_at, "magic_str");
  });
}

