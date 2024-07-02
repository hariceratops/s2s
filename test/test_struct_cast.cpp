#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include "../single_header/struct_cast.hpp"
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <cassert>


// Helper types
using i32 = int;
using u32 = unsigned int;
using u8 = unsigned char;


TEST_CASE("Test reading a meta_struct from a static buffer") {
  using test_struct_field_list = 
    struct_field_list<
      field<"a", u32, 4>, 
      field<"b", u32, 4>
    >;

  const u8 buffer[] = {
    0xef, 0xbe, 0xad, 0xde,
    0x0d, 0xd0, 0xfe, 0xca,
    0xef, 0xbe, 0xef, 0xbe
  };
  test_struct_field_list fields;

  struct_cast(fields, buffer);

  REQUIRE(fields["a"_f] == 0xdeadbeef);
  REQUIRE(fields["b"_f] == 0xcafed00d);
}


TEST_CASE("Test reading a meta_struct from a binary file") {
  using test_struct_field_list = 
   struct_field_list<
     field<"a", u32, 4>, 
     field<"b", u32, 4>
  >;

  std::ofstream ofs("test_bin_input_1.bin", std::ios::out | std::ios::binary);
  u32 a = 0xdeadbeef;
  u32 b = 0xcafed00d;
  ofs.write(reinterpret_cast<const char*>(&a), sizeof(a));
  ofs.write(reinterpret_cast<const char*>(&b), sizeof(a));
  ofs.close();

  std::ifstream ifs("test_bin_input_1.bin", std::ios::in | std::ios::binary);
  test_struct_field_list fields;
  struct_cast(fields, ifs);
  ifs.close();

  REQUIRE(fields["a"_f] == 0xdeadbeef);
  REQUIRE(fields["b"_f] == 0xcafed00d);
}


TEST_CASE("Test reading a meta_struct with nested struct from a static buffer") {
  using test_nested_struct_field_list = 
    struct_field_list<
      field<"a", u32, 4>, 
      field<"b", u32, 4>,
      struct_field<
        "c", 
        struct_field_list<
          field<"x", u32, 4>,
          field<"y", u32, 4>
        >
      >
    >;

  const u8 buffer[] = {
    0xef, 0xbe, 0xad, 0xde,
    0x0d, 0xd0, 0xfe, 0xca,
    0xef, 0xbe, 0xef, 0xbe,
    0xef, 0xbe, 0xad, 0xde
  };
  test_nested_struct_field_list fields;

  struct_cast(fields, buffer);

  REQUIRE(fields["a"_f] == 0xdeadbeef);
  REQUIRE(fields["b"_f] == 0xcafed00d);
  REQUIRE(fields["c"_f]["x"_f] == 0xbeefbeef);
  REQUIRE(fields["c"_f]["y"_f] == 0xdeadbeef);
}


TEST_CASE("Test reading a meta_struct with nested struct from a binary file") {
   using test_nested_struct_field_list = 
    struct_field_list<
      field<"a", u32, 4>, 
      field<"b", u32, 4>,
      struct_field<
        "c", 
        struct_field_list<
          field<"x", u32, 4>,
          field<"y", u32, 4>
        >
      >
    >;

  std::ofstream ofs("test_bin_input_2.bin", std::ios::out | std::ios::binary);
  u32 a = 0xdeadbeef;
  u32 b = 0xcafed00d;
  u32 x = 0xbeefbeef;
  u32 y = 0xdeadbeef;

  ofs.write(reinterpret_cast<const char*>(&a), sizeof(a));
  ofs.write(reinterpret_cast<const char*>(&b), sizeof(a));
  ofs.write(reinterpret_cast<const char*>(&x), sizeof(a));
  ofs.write(reinterpret_cast<const char*>(&y), sizeof(a));
  ofs.close();

  std::ifstream ifs("test_bin_input_2.bin", std::ios::in | std::ios::binary);
  test_nested_struct_field_list fields;
  struct_cast(fields, ifs);
  ifs.close();

  REQUIRE(fields["a"_f] == 0xdeadbeef);
  REQUIRE(fields["b"_f] == 0xcafed00d);
  REQUIRE(fields["c"_f]["x"_f] == 0xbeefbeef);
  REQUIRE(fields["c"_f]["y"_f] == 0xdeadbeef);
}


TEST_CASE("Test reading a meta_struct with fixed buffer fields from binary file") {
  using test_fixed_buffer_struct = 
    struct_field_list<
      field<"a", char[11], 11>,
      field<"b", fixed_string<11>, 11>,
      field<"c", std::array<u32, 3>, 12>
    >;

  constexpr std::size_t str_len = 10;
  const u8 str[] = "foo in bar";
  const u32 u32_arr[] = {0xdeadbeef, 0xcafed00d, 0xbeefbeef};

  std::ofstream ofs("test_bin_input_3.bin", std::ios::out | std::ios::binary);
  // c_str of length 10
  ofs.write(reinterpret_cast<const char*>(&str), str_len + 1);
  // fixed_string of length 10
  ofs.write(reinterpret_cast<const char*>(&str), str_len + 1);
  // array of integers of length 3
  ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
  ofs.close();

  std::ifstream ifs("test_bin_input_3.bin", std::ios::in | std::ios::binary);
  test_fixed_buffer_struct fields;
  struct_cast(fields, ifs);
  ifs.close();
  
  std::string_view expected{"foo in bar"};
  REQUIRE(std::string_view{fields["a"_f]} == expected);
  REQUIRE(std::string_view{fields["b"_f].data()} == expected);
  REQUIRE(fields["c"_f] == std::array<u32, 3>{0xdeadbeef, 0xcafed00d, 0xbeefbeef});
};


TEST_CASE("Test reading a meta_struct with aliased fixed buffer fields from binary file") {
  using test_aliased_fixed_buffer_struct = 
    struct_field_list<
      c_str_field<"a", 10>,
      fixed_string_field<"b", 10>,
      fixed_array_field<"c", u32, 3>
    >;

  constexpr std::size_t str_len = 10;
  const u8 str[] = "foo in bar";
  const u32 u32_arr[] = {0xdeadbeef, 0xcafed00d, 0xbeefbeef};

  std::ofstream ofs("test_bin_input_4.bin", std::ios::out | std::ios::binary);
  ofs.write(reinterpret_cast<const char*>(&str), str_len + 1);
  ofs.write(reinterpret_cast<const char*>(&str), str_len + 1);
  ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
  ofs.close();

  std::ifstream ifs("test_bin_input_4.bin", std::ios::in | std::ios::binary);
  test_aliased_fixed_buffer_struct fields;
  struct_cast(fields, ifs);
  ifs.close();

  std::string_view expected{"foo in bar"};
  REQUIRE(std::string_view{fields["a"_f]} == expected);
  REQUIRE(std::string_view{fields["b"_f].data()} == expected);
  REQUIRE(fields["c"_f] == std::array<u32, 3>{0xdeadbeef, 0xcafed00d, 0xbeefbeef});
};


TEST_CASE("Test reading a meta_struct with multidimensional fixed buffer field from binary file") {
  using md_struct = 
    struct_field_list<
      fixed_array_field<"arr", std::array<u32, 3>, 3>
    >;

  std::ofstream ofs("test_bin_input_5.bin", std::ios::out | std::ios::binary);
  const u32 u32_arr[3][3] = { 
    {0xdeadbeef, 0xcafed00d, 0xbeefbeef},
    {0xdeadbeef, 0xcafed00d, 0xbeefbeef}, 
    {0xdeadbeef, 0xcafed00d, 0xbeefbeef} 
  };
  ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
  ofs.close();

  std::ifstream ifs("test_bin_input_5.bin", std::ios::in | std::ios::binary);
  md_struct fields;
  struct_cast(fields, ifs);
  ifs.close();
  
  REQUIRE(
    fields["arr"_f] == 
    std::array<std::array<u32, 3>, 3> {{
      {0xdeadbeef, 0xcafed00d, 0xbeefbeef},
      {0xdeadbeef, 0xcafed00d, 0xbeefbeef},
      {0xdeadbeef, 0xcafed00d, 0xbeefbeef}
    }});
};


TEST_CASE("Test reading a meta_struct with array of records from binary file") {
  using test_struct = 
    struct_field_list <
      field<"a", u32, 4>,
      field<"b", u32, 4>
    >;
  using md_struct = 
    struct_field_list<
      fixed_array_field<"records", test_struct, 3>
    >;

  std::ofstream ofs("test_bin_input_6.bin", std::ios::out | std::ios::binary);
  const u32 u32_arr[3][2] = { 
    {0xdeadbeef, 0xbeefbeef},
    {0xdeadbeef, 0xbeefbeef}, 
    {0xdeadbeef, 0xbeefbeef} 
  };
  ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
  ofs.close();

  std::ifstream ifs("test_bin_input_6.bin", std::ios::in | std::ios::binary);
  md_struct fields;
  struct_cast(fields, ifs);
  ifs.close();
  
  auto records = fields["records"_f];
  for(auto record: records) {
    REQUIRE(record["a"_f] == 0xdeadbeef);
    REQUIRE(record["b"_f] == 0xbeefbeef);
  }
};

