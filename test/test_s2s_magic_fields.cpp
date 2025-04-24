#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include "../single_header/struct_cast.hpp"
#include "s2s_test_utils.hpp"


using namespace s2s_literals;


TEST_CASE("Test magic number") {
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
    REQUIRE(result.has_value());
    auto fields = *result;
    REQUIRE(fields["magic_num"_f] == 0xdeadbeef);
    REQUIRE(fields["p"_f] == 0xcafed00d);
  });
}


TEST_CASE("Test magic array") {
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
    REQUIRE(result.has_value());
    auto fields = *result;
    REQUIRE(fields["magic_arr"_f] == std::array<u8, 10>{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff});
    REQUIRE(fields["size"_f] == 0xcafed00d);
  });
}


TEST_CASE("Test magic string") {
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
    REQUIRE(result.has_value());
    auto fields = *result;
    REQUIRE(std::string_view{fields["magic_str"_f].data()} == std::string_view{s2s::fixed_string("GIF").data()});
    REQUIRE(fields["size"_f] == 0xcafed00d);
  });
}


TEST_CASE("Test failing magic string read") {
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
    REQUIRE(!result.has_value());
    auto err = result.error();
    REQUIRE(err.failure_reason == s2s::error_reason::validation_failure);
    REQUIRE(err.failed_at == "magic_str");
  });
}


