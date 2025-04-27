#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include "../single_header/struct_cast.hpp"
#include "s2s_test_utils.hpp"


using namespace s2s_literals;


TEST_CASE("Test reading a meta_struct with s2s::fixed buffer fields from binary file") {
  [](){
    std::ofstream file("test_input.bin", std::ios::out | std::ios::binary); \
    constexpr std::size_t str_len = 10;
    const u8 str[] = "foo in bar";
    const u32 u32_arr[] = {0xdeadbeef, 0xcafed00d, 0xbeefbeef};
    file.write(reinterpret_cast<const char*>(&str), str_len + 1);
    file.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
  }();

  FIELD_LIST_SCHEMA = 
    s2s::struct_field_list<
      // c_s2s::str_field<"a", 10>,
      s2s::fixed_string_field<"b", 10>,
      s2s::fixed_array_field<"c", u32, 3>
    >;

  FIELD_LIST_LE_READ_CHECK({
    std::string_view expected{"foo in bar"};
    REQUIRE(result.has_value() == true);
    auto fields = *result;
    // REQUIRE(std::string_view{fields["a"_f]} == expected);
    REQUIRE(std::string_view{fields["b"_f].data()} == expected);
    REQUIRE(fields["c"_f] == std::array<u32, 3>{0xdeadbeef, 0xcafed00d, 0xbeefbeef});
  });
};


TEST_CASE("Test reading a meta_struct with multidimensional s2s::fixed buffer field from binary file") {
  []() {
    std::ofstream ofs("test_input.bin", std::ios::out | std::ios::binary);
    const u32 u32_arr[3][3] = { 
      {0xdeadbeef, 0xcafed00d, 0xbeefbeef},
      {0xdeadbeef, 0xcafed00d, 0xbeefbeef}, 
      {0xdeadbeef, 0xcafed00d, 0xbeefbeef} 
    };
    ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
  }();

  FIELD_LIST_SCHEMA = 
    s2s::struct_field_list<
      s2s::fixed_array_field<"arr", std::array<u32, 3>, 3>
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value());
    auto fields = *result;
    REQUIRE(
      fields["arr"_f] == 
      std::array<std::array<u32, 3>, 3> {{
        {0xdeadbeef, 0xcafed00d, 0xbeefbeef},
        {0xdeadbeef, 0xcafed00d, 0xbeefbeef},
        {0xdeadbeef, 0xcafed00d, 0xbeefbeef}
      }});
  });
};





