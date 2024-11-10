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
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>
    >;

  const u8 buffer[] = {
    0xef, 0xbe, 0xad, 0xde,
    0x0d, 0xd0, 0xfe, 0xca,
    0xef, 0xbe, 0xef, 0xbe
  };

  auto result = struct_cast<test_struct_field_list>(buffer);

  REQUIRE(result.has_value() == true);
  if(result) {
    auto fields = *result;
    REQUIRE(fields["a"_f] == 0xdeadbeef);
    REQUIRE(fields["b"_f] == 0xcafed00d);
  }
}


// TEST_CASE("Test reading a meta_struct from a binary file") {
//   using test_struct_field_list = 
//    struct_field_list<
//      field<"a", u32, field_size<4>>, 
//      field<"b", u32, field_size<4>>
//   >;
//
//   std::ofstream ofs("test_bin_input_1.bin", std::ios::out | std::ios::binary);
//   u32 a = 0xdeadbeef;
//   u32 b = 0xcafed00d;
//   ofs.write(reinterpret_cast<const char*>(&a), sizeof(a));
//   ofs.write(reinterpret_cast<const char*>(&b), sizeof(a));
//   ofs.close();
//
//   std::ifstream ifs("test_bin_input_1.bin", std::ios::in | std::ios::binary);
//   test_struct_field_list fields;
//   struct_cast(fields, ifs);
//   ifs.close();
//
//   REQUIRE(fields["a"_f] == 0xdeadbeef);
//   REQUIRE(fields["b"_f] == 0xcafed00d);
// }
//
//
TEST_CASE("Test reading a meta_struct with nested struct from a static buffer") {
  using test_nested_struct_field_list = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      struct_field<
        "c", 
        struct_field_list<
          basic_field<"x", u32, field_size<fixed<4>>>,
          basic_field<"y", u32, field_size<fixed<4>>>
        >
      >
    >;

  const u8 buffer[] = {
    0xef, 0xbe, 0xad, 0xde,
    0x0d, 0xd0, 0xfe, 0xca,
    0xef, 0xbe, 0xef, 0xbe,
    0xef, 0xbe, 0xad, 0xde
  };

  auto result = struct_cast<test_nested_struct_field_list>(buffer);

  REQUIRE(result.has_value() == true);
  if(result) {
    auto fields = *result;
    REQUIRE(fields["a"_f] == 0xdeadbeef);
    REQUIRE(fields["b"_f] == 0xcafed00d);
    REQUIRE(fields["c"_f]["x"_f] == 0xbeefbeef);
    REQUIRE(fields["c"_f]["y"_f] == 0xdeadbeef);
  }
}
//
//
// TEST_CASE("Test reading a meta_struct with nested struct from a binary file") {
//    using test_nested_struct_field_list = 
//     struct_field_list<
//       field<"a", u32, field_size<4>>, 
//       field<"b", u32, field_size<4>>,
//       struct_field<
//         "c", 
//         struct_field_list<
//           field<"x", u32, field_size<4>>,
//           field<"y", u32, field_size<4>>
//         >
//       >
//     >;
//
//   std::ofstream ofs("test_bin_input_2.bin", std::ios::out | std::ios::binary);
//   u32 a = 0xdeadbeef;
//   u32 b = 0xcafed00d;
//   u32 x = 0xbeefbeef;
//   u32 y = 0xdeadbeef;
//
//   ofs.write(reinterpret_cast<const char*>(&a), sizeof(a));
//   ofs.write(reinterpret_cast<const char*>(&b), sizeof(a));
//   ofs.write(reinterpret_cast<const char*>(&x), sizeof(a));
//   ofs.write(reinterpret_cast<const char*>(&y), sizeof(a));
//   ofs.close();
//
//   std::ifstream ifs("test_bin_input_2.bin", std::ios::in | std::ios::binary);
//   test_nested_struct_field_list fields;
//   struct_cast(fields, ifs);
//   ifs.close();
//
//   REQUIRE(fields["a"_f] == 0xdeadbeef);
//   REQUIRE(fields["b"_f] == 0xcafed00d);
//   REQUIRE(fields["c"_f]["x"_f] == 0xbeefbeef);
//   REQUIRE(fields["c"_f]["y"_f] == 0xdeadbeef);
// }
//
//
// TEST_CASE("Test reading a meta_struct with fixed buffer fields from binary file") {
//   using test_fixed_buffer_struct = 
//     struct_field_list<
//       field<"a", char[11], field_size<11>>,
//       field<"b", fixed_string<11>, field_size<11>>,
//       field<"c", std::array<u32, 3>, field_size<12>>
//     >;
//
//   constexpr std::size_t str_len = 10;
//   const u8 str[] = "foo in bar";
//   const u32 u32_arr[] = {0xdeadbeef, 0xcafed00d, 0xbeefbeef};
//
//   std::ofstream ofs("test_bin_input_3.bin", std::ios::out | std::ios::binary);
//   // c_str of length 10
//   ofs.write(reinterpret_cast<const char*>(&str), str_len + 1);
//   // fixed_string of length 10
//   ofs.write(reinterpret_cast<const char*>(&str), str_len + 1);
//   // array of integers of length 3
//   ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
//   ofs.close();
//
//   std::ifstream ifs("test_bin_input_3.bin", std::ios::in | std::ios::binary);
//   test_fixed_buffer_struct fields;
//   struct_cast(fields, ifs);
//   ifs.close();
//   
//   std::string_view expected{"foo in bar"};
//   REQUIRE(std::string_view{fields["a"_f]} == expected);
//   REQUIRE(std::string_view{fields["b"_f].data()} == expected);
//   REQUIRE(fields["c"_f] == std::array<u32, 3>{0xdeadbeef, 0xcafed00d, 0xbeefbeef});
// };
//
//
// TEST_CASE("Test reading a meta_struct with aliased fixed buffer fields from binary file") {
//   using test_aliased_fixed_buffer_struct = 
//     struct_field_list<
//       c_str_field<"a", 10>,
//       fixed_string_field<"b", 10>,
//       fixed_array_field<"c", u32, 3>
//     >;
//
//   constexpr std::size_t str_len = 10;
//   const u8 str[] = "foo in bar";
//   const u32 u32_arr[] = {0xdeadbeef, 0xcafed00d, 0xbeefbeef};
//
//   std::ofstream ofs("test_bin_input_4.bin", std::ios::out | std::ios::binary);
//   ofs.write(reinterpret_cast<const char*>(&str), str_len + 1);
//   ofs.write(reinterpret_cast<const char*>(&str), str_len + 1);
//   ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
//   ofs.close();
//
//   std::ifstream ifs("test_bin_input_4.bin", std::ios::in | std::ios::binary);
//   test_aliased_fixed_buffer_struct fields;
//   struct_cast(fields, ifs);
//   ifs.close();
//
//   std::string_view expected{"foo in bar"};
//   REQUIRE(std::string_view{fields["a"_f]} == expected);
//   REQUIRE(std::string_view{fields["b"_f].data()} == expected);
//   REQUIRE(fields["c"_f] == std::array<u32, 3>{0xdeadbeef, 0xcafed00d, 0xbeefbeef});
// };
//
//
// TEST_CASE("Test reading a meta_struct with multidimensional fixed buffer field from binary file") {
//   using md_struct = 
//     struct_field_list<
//       fixed_array_field<"arr", std::array<u32, 3>, 3>
//     >;
//
//   std::ofstream ofs("test_bin_input_5.bin", std::ios::out | std::ios::binary);
//   const u32 u32_arr[3][3] = { 
//     {0xdeadbeef, 0xcafed00d, 0xbeefbeef},
//     {0xdeadbeef, 0xcafed00d, 0xbeefbeef}, 
//     {0xdeadbeef, 0xcafed00d, 0xbeefbeef} 
//   };
//   ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
//   ofs.close();
//
//   std::ifstream ifs("test_bin_input_5.bin", std::ios::in | std::ios::binary);
//   md_struct fields;
//   struct_cast(fields, ifs);
//   ifs.close();
//   
//   REQUIRE(
//     fields["arr"_f] == 
//     std::array<std::array<u32, 3>, 3> {{
//       {0xdeadbeef, 0xcafed00d, 0xbeefbeef},
//       {0xdeadbeef, 0xcafed00d, 0xbeefbeef},
//       {0xdeadbeef, 0xcafed00d, 0xbeefbeef}
//     }});
// };
//
//
// TEST_CASE("Test reading a meta_struct with array of records from binary file") {
//   using test_struct = 
//     struct_field_list <
//       field<"a", u32, field_size<4>>,
//       field<"b", u32, field_size<4>>
//     >;
//   using md_struct = 
//     struct_field_list<
//       fixed_array_field<"records", test_struct, 3>
//     >;
//
//   std::ofstream ofs("test_bin_input_6.bin", std::ios::out | std::ios::binary);
//   const u32 u32_arr[3][2] = { 
//     {0xdeadbeef, 0xbeefbeef},
//     {0xdeadbeef, 0xbeefbeef}, 
//     {0xdeadbeef, 0xbeefbeef} 
//   };
//   ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
//   ofs.close();
//
//   std::ifstream ifs("test_bin_input_6.bin", std::ios::in | std::ios::binary);
//   md_struct fields;
//   struct_cast(fields, ifs);
//   ifs.close();
//   
//   auto records = fields["records"_f];
//   for(auto record: records) {
//     REQUIRE(record["a"_f] == 0xdeadbeef);
//     REQUIRE(record["b"_f] == 0xbeefbeef);
//   }
// };
//
//
// TEST_CASE("Test eq field constraint") {
//   auto eq_obj = eq(42);
//   REQUIRE(eq_obj(42) == true);
//   REQUIRE(eq_obj(84) == false);
// }
//
//
// TEST_CASE("Test neq field constraint") {
//   auto neq_obj = neq(42);
//   REQUIRE(neq_obj(42) == false);
//   REQUIRE(neq_obj(84) == true);
// }
//
//
// TEST_CASE("Test lt field constraint") {
//   auto lt_obj = lt(42);
//   REQUIRE(lt_obj(21) == true);
//   REQUIRE(lt_obj(84) == false);
// }
//
//
// TEST_CASE("Test gt field constraint") {
//   auto gt_obj = gt(42);
//   REQUIRE(gt_obj(21) == false);
//   REQUIRE(gt_obj(84) == true);
// }
//
//
// TEST_CASE("Test lte field constraint") {
//   auto lte_obj = lte(42);
//   REQUIRE(lte_obj(21) == true);
//   REQUIRE(lte_obj(42) == true);
//   REQUIRE(lte_obj(84) == false);
// }
//
//
// TEST_CASE("Test gte field constraint") {
//   auto gte_obj = gte(42);
//   REQUIRE(gte_obj(21) == false);
//   REQUIRE(gte_obj(42) == true);
//   REQUIRE(gte_obj(84) == true);
// }
//
//
// TEST_CASE("Test any_of field constraint") {
//   auto any_of_obj = any_of(21, 42, 84);
//   REQUIRE(any_of_obj(21) == true);
//   REQUIRE(any_of_obj(42) == true);
//   REQUIRE(any_of_obj(84) == true);
//   REQUIRE(any_of_obj(100) == false);
// }
//
//
// TEST_CASE("Test open range interval check") {
//   // constexpr auto ranges = std::array<range<u32>, 3>{range{2u, 4u}, range{6u, 9u}, range{13u, 15u}};
//   // constexpr auto r = range{2u, 4u};
// }
//
//
// TEST_CASE("Test magic number") {
//   using test_struct_field_list = 
//     struct_field_list<
//       magic_number<"magic_num", u32, 4, 0xdeadbeef>,
//       field<"p", u32, field_size<4>>
//     >;
//   const u8 buffer[] = {
//     0xef, 0xbe, 0xad, 0xde,
//     0x0d, 0xd0, 0xfe, 0xca,
//     0xef, 0xbe, 0xef, 0xbe
//   };
//   test_struct_field_list fields;
//
//   struct_cast(fields, buffer);
//
//   REQUIRE(fields["magic_num"_f] == 0xdeadbeef);
//   REQUIRE(fields["p"_f] == 0xcafed00d);
// }
//
//
// TEST_CASE("Test magic array") {
//   using test_struct_field_list = 
//     struct_field_list<
//       magic_byte_array<"magic_arr", 10, std::array<unsigned char, 10>{0xff, 0xff, 0xff, 0xff, 0xff, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd}>,
//       field<"size", u32, field_size<4>>
//     >;
//   const u8 buffer[] = {
//     0xff, 0xff, 0xff, 0xff, 0xff, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
//     0x0d, 0xd0, 0xfe, 0xca
//   };
//   test_struct_field_list fields;
//
//   struct_cast(fields, buffer);
//
//   REQUIRE(fields["magic_arr"_f] == std::array<u8, 10>{0xff, 0xff, 0xff, 0xff, 0xff, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd});
//   REQUIRE(fields["size"_f] == 0xcafed00d);
// }
//
//
// TEST_CASE("Test magic string") {
//   using test_struct_field_list = 
//     struct_field_list<
//       magic_string<"magic_str", "GIF">,
//       field<"size", u32, field_size<4>>
//     >;
//   const u8 buffer[] = {
//     'G', 'I', 'F',
//     0x0d, 0xd0, 0xfe, 0xca
//   };
//   test_struct_field_list fields;
//
//   struct_cast(fields, buffer);
//
//   REQUIRE(std::string_view{fields["magic_str"_f].data()} == std::string_view{fixed_string("GIF").data()});
//   REQUIRE(fields["size"_f] == 0xcafed00d);
// }
//
//
// TEST_CASE("Test reading a meta_struct with aliased length prefixed buffer fields from binary file") {
//   using test_aliased_var_buffer_struct = 
//     struct_field_list<
//       field<"len", std::size_t, field_size<8>>,
//       vec_field<"vec", int, runtime_size<from_field<"len">>>
//       // str_field<"str", runtime_size<from_field<"len">>>
//     >;
//
//   constexpr std::size_t str_len = 40;
//   const u8 str[] = "foo in bar";
//   const u32 u32_arr[] = {
//     0xdeadbeef, 0xcafed00d,
//     0xdeadbeef, 0xcafed00d,
//     0xdeadbeef, 0xcafed00d,
//     0xdeadbeef, 0xcafed00d,
//     0xdeadbeef, 0xcafed00d
//   };
//
//   std::ofstream ofs("test_bin_input_4.bin", std::ios::out | std::ios::binary);
//   ofs.write(reinterpret_cast<const char*>(&str_len), sizeof(str_len));
//   ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
//   ofs.write(reinterpret_cast<const char*>(&str), str_len + 1);
//   ofs.close();
//
//   std::ifstream ifs("test_bin_input_4.bin", std::ios::in | std::ios::binary);
//   test_aliased_var_buffer_struct fields;
//   struct_cast(fields, ifs);
//   ifs.close();
//   
//   std::cout << fields["len"_f] << '\n';
//   std::cout << fields["vec"_f].size() << '\n';
//   for(auto num: fields["vec"_f]) {
//     std::cout << std::hex << num << '\n';
//   }
//
//   // std::string_view expected{"foo in bar"};
//   // REQUIRE(std::string_view{fields["a"_f]} == expected);
//   // REQUIRE(std::string_view{fields["b"_f].data()} == expected);
//   // REQUIRE(fields["c"_f] == std::array<u32, 3>{0xdeadbeef, 0xcafed00d, 0xbeefbeef});
// };
//
//
// TEST_CASE("Dummy test to verify runtime computation from fields") {
//   using u32 = unsigned int;
//   using sfl = struct_field_list<field<"a", u32, field_size<4>>, field<"b", u32, field_size<4>>>;
//   sfl fl;
//   unsigned char arr[] = {0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00};
//   struct_cast(fl, arr);
//
//   auto callable = [](u32& a, u32& b) -> u32 { return a * b; };
//   using dep_fields = 
//     typelist::typelist<
//       field<"a", u32, field_size<4>>,
//       field<"b", u32, field_size<4>>
//     >;
//   auto res = compute<callable, dep_fields>()(fl);
//
//   REQUIRE(fl["a"_f] == 4);
//   REQUIRE(fl["b"_f] == 5);
//   REQUIRE(res == 20);
//   std::cout << std::dec << res << '\n';
// }
//
//

namespace static_test {
  auto unit = [](auto a){ return a * 1; };
  auto is_a_eq_1 = [](auto a){ return a == 1; };
  using test_struct_field_list = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>
    >;
  static_assert(is_invocable<is_a_eq_1, bool, test_struct_field_list, with_fields<"a">>::res);
  static_assert(can_eval_R_from_fields<is_a_eq_1, int, test_struct_field_list, with_fields<"a">>);
  // static_assert(can_eval_R_from_fields<is_a_eq_1, char*, test_struct_field_list, with_fields<"a">>);
  static_assert(is_invocable<unit, bool, test_struct_field_list, with_fields<"a">>::res);
}

/*
*    auto constexpr unit = [](int a)-> int { return a * 1; };
    static_assert(std::is_invocable_r_v<int, decltype(unit), int>);
    static_assert(std::is_invocable_r_v<float, decltype(unit), int>);
    static_assert(std::is_invocable_r_v<char*, decltype(unit), int>);*/

TEST_CASE("Test case to verify option field parsing") {
  auto is_a_eq_1 = [](auto a){ return a == 1; };

  using test_struct_field_list = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      maybe_field<"c", u32, field_size<fixed<4>>, parse_if<is_a_eq_1, with_fields<"a">>>
    >;

  const u8 buffer[] = {
    0xef, 0xbe, 0xad, 0xde,
    0x0d, 0xd0, 0xfe, 0xca,
    0xef, 0xbe, 0xef, 0xbe
  };

  auto result = struct_cast<test_struct_field_list>(buffer);

  REQUIRE(result.has_value() == true);
  if(result) {
    auto fields = *result;
    REQUIRE(fields["a"_f] == 0xdeadbeef);
    REQUIRE(fields["b"_f] == 0xcafed00d);
    REQUIRE(fields["c"_f] == std::nullopt);
  }
}

TEST_CASE("Test case to verify variant field parsing") {
  // todo a convinient type to get field "x"
  auto get_a = [](auto a){ return a; };

  // todo introduce type named expression to avoid decltype
  using test_struct_field_list = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      union_field<
        "c", 
        type<
          decltype(get_a),
          type_switch<
            match_case<0xdeadbeef, type_tag<int, field_size<fixed<4>>>>,
            match_case<0xcafed00d, type_tag<float, field_size<fixed<4>>>>,
            match_case<0xbeefbeef, type_tag<unsigned int, field_size<fixed<4>>>>
          >
        >
      >
    >;

  const u8 buffer[] = {
    0xef, 0xbe, 0xad, 0xde,
    0x0d, 0xd0, 0xfe, 0xca,
    0xef, 0xbe, 0xef, 0xbe
  };

  auto result = struct_cast<test_struct_field_list>(buffer);

  REQUIRE(result.has_value() == true);
  if(result) {
    auto fields = *result;
    std::cout << fields["a"_f];
    REQUIRE(fields["a"_f] == 0xdeadbeef);
    REQUIRE(fields["b"_f] == 0xcafed00d);
  }
}


