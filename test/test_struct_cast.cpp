#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include "../single_header/struct_cast.hpp"
#include <cstdlib>
#include <iostream>
#include <cassert>


// todo static tests
//// namespace static_test {
//   // implicit conversion produces false positive in second static_assert, 
//   // must be checked with std::is_same_v additionally
//   // auto constexpr unit = [](int a)-> int { return a * 1; };
//   // static_assert(std::is_invocable_r_v<int, decltype(unit), int>);
//   // static_assert(std::is_invocable_r_v<float, decltype(unit), int>);
//   // static_assert(std::is_invocable_r_v<char*, decltype(unit), int>);
//
//   auto unit = [](auto a){ return a * 1; };
//   auto is_a_eq_1 = [](auto a){ return a == 1; };
//   using test_struct_field_list = 
//     struct_field_list<
//       basic_field<"a", u32, field_size<fixed<4>>>, 
//       basic_field<"b", u32, field_size<fixed<4>>>
//     >;
//   static_assert(is_invocable<is_a_eq_1, bool, test_struct_field_list, with_fields<"a">>::res);
//   static_assert(can_eval_R_from_fields<is_a_eq_1, int, test_struct_field_list, with_fields<"a">>);
//   // static_assert(can_eval_R_from_fields<is_a_eq_1, char*, test_struct_field_list, with_fields<"a">>);
//   static_assert(is_invocable<unit, bool, test_struct_field_list, with_fields<"a">>::res);
// }
  // using inner =  
  //   struct_field<
  //       "c", 
  //       struct_field_list<
  //         basic_field<"x", u32, field_size<fixed<4>>>,
  //         basic_field<"y", u32, field_size<fixed<4>>>
  //       >
  //     >;
  // static_assert(struct_field_like<inner>);
  // static_assert(variable_sized_field_like<inner>);
  // static_assert(fixed_sized_field_like<inner>);


// Helper types
using i32 = int;
using u32 = unsigned int;
using u8 = unsigned char;
using u16 = unsigned short;


TEST_CASE("Test reading a meta_struct from a binary file") {
  using test_struct_field_list = 
   struct_field_list<
     basic_field<"a", u32, field_size<fixed<4>>>, 
     basic_field<"b", u32, field_size<fixed<4>>>
  >;

  std::ofstream ofs("test_bin_input_1.bin", std::ios::out | std::ios::binary);
  u32 a = 0xdeadbeef;
  u32 b = 0xcafed00d;
  ofs.write(reinterpret_cast<const char*>(&a), sizeof(a));
  ofs.write(reinterpret_cast<const char*>(&b), sizeof(b));
  ofs.close();

  std::ifstream ifs("test_bin_input_1.bin", std::ios::in | std::ios::binary);

  auto result = struct_cast<test_struct_field_list>(ifs);
  ifs.close();

  REQUIRE(result.has_value() == true);
  if(result) {
    auto fields = *result;
    REQUIRE(fields["a"_f] == 0xdeadbeef);
    REQUIRE(fields["b"_f] == 0xcafed00d);
  }
}


TEST_CASE("Test reading a meta_struct with nested struct from a binary file") {
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

  std::ofstream ofs("test_bin_input_2.bin", std::ios::out | std::ios::binary);
  u32 a = 0xdeadbeef;
  u32 b = 0xcafed00d;
  u32 x = 0xbeefbeef;
  u32 y = 0xdeadbeef;

  ofs.write(reinterpret_cast<const char*>(&a), sizeof(a));
  ofs.write(reinterpret_cast<const char*>(&b), sizeof(b));
  ofs.write(reinterpret_cast<const char*>(&x), sizeof(x));
  ofs.write(reinterpret_cast<const char*>(&y), sizeof(y));
  ofs.close();

  std::ifstream ifs("test_bin_input_2.bin", std::ios::in | std::ios::binary);
  auto result = struct_cast<test_nested_struct_field_list>(ifs);
  ifs.close();

  REQUIRE(result.has_value() == true);
  if(result) {
    auto fields = *result;
    REQUIRE(fields["a"_f] == 0xdeadbeef);
    REQUIRE(fields["b"_f] == 0xcafed00d);
    REQUIRE(fields["c"_f]["x"_f] == 0xbeefbeef);
    REQUIRE(fields["c"_f]["y"_f] == 0xdeadbeef);
  }
}


TEST_CASE("Test reading a meta_struct with fixed buffer fields from binary file") {
  using test_fixed_buffer_struct = 
    struct_field_list<
      // c_str_field<"a", 10>,
      fixed_string_field<"b", 10>,
      fixed_array_field<"c", u32, 3>
    >;

  constexpr std::size_t str_len = 10;
  const u8 str[] = "foo in bar";
  const u32 u32_arr[] = {0xdeadbeef, 0xcafed00d, 0xbeefbeef};

  std::ofstream ofs("test_bin_input_3.bin", std::ios::out | std::ios::binary);
  // c_str of length 10
  // ofs.write(reinterpret_cast<const char*>(&str), str_len + 1);
  // fixed_string of length 10
  ofs.write(reinterpret_cast<const char*>(&str), str_len + 1);
  // array of integers of length 3
  ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
  ofs.close();

  std::ifstream ifs("test_bin_input_3.bin", std::ios::in | std::ios::binary);
  auto res = struct_cast<test_fixed_buffer_struct>(ifs);
  ifs.close();
  
  std::string_view expected{"foo in bar"};
  REQUIRE(res.has_value() == true);
  auto fields = *res;
  // REQUIRE(std::string_view{fields["a"_f]} == expected);
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
  auto res = struct_cast<md_struct>(ifs);
  ifs.close();
  
  REQUIRE(res.has_value());
  auto fields = *res;
  REQUIRE(
    fields["arr"_f] == 
    std::array<std::array<u32, 3>, 3> {{
      {0xdeadbeef, 0xcafed00d, 0xbeefbeef},
      {0xdeadbeef, 0xcafed00d, 0xbeefbeef},
      {0xdeadbeef, 0xcafed00d, 0xbeefbeef}
    }});
};


// todo array of records and vector of records
// TEST_CASE("Test reading a meta_struct with array of records from binary file") {
//   using test_struct = 
//     struct_field_list <
//       basic_field<"a", u32, field_size<fixed<4>>>,
//       basic_field<"b", u32, field_size<fixed<4>>>
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
//   auto res = struct_cast<md_struct>(ifs);
//   ifs.close();
//   
//   REQUIRE(res.has_value());
//   auto fields = *res;
//   auto records = fields["records"_f];
//   for(auto record: records) {
//     REQUIRE(record["a"_f] == 0xdeadbeef);
//     REQUIRE(record["b"_f] == 0xbeefbeef);
//   }
// };


TEST_CASE("Test eq field constraint") {
  auto eq_obj = eq(42);
  REQUIRE(eq_obj(42) == true);
  REQUIRE(eq_obj(84) == false);
}


TEST_CASE("Test neq field constraint") {
  auto neq_obj = neq(42);
  REQUIRE(neq_obj(42) == false);
  REQUIRE(neq_obj(84) == true);
}


TEST_CASE("Test lt field constraint") {
  auto lt_obj = lt(42);
  REQUIRE(lt_obj(21) == true);
  REQUIRE(lt_obj(84) == false);
}


TEST_CASE("Test gt field constraint") {
  auto gt_obj = gt(42);
  REQUIRE(gt_obj(21) == false);
  REQUIRE(gt_obj(84) == true);
}


TEST_CASE("Test lte field constraint") {
  auto lte_obj = lte(42);
  REQUIRE(lte_obj(21) == true);
  REQUIRE(lte_obj(42) == true);
  REQUIRE(lte_obj(84) == false);
}


TEST_CASE("Test gte field constraint") {
  auto gte_obj = gte(42);
  REQUIRE(gte_obj(21) == false);
  REQUIRE(gte_obj(42) == true);
  REQUIRE(gte_obj(84) == true);
}


TEST_CASE("Test any_of field constraint") {
  auto any_of_obj = any_of(21, 42, 84);
  REQUIRE(any_of_obj(21) == true);
  REQUIRE(any_of_obj(42) == true);
  REQUIRE(any_of_obj(84) == true);
  REQUIRE(any_of_obj(100) == false);
}


TEST_CASE("Test open range interval check") {
  // constexpr auto ranges = std::array<range<u32>, 3>{range{2u, 4u}, range{6u, 9u}, range{13u, 15u}};
  // constexpr auto r = range{2u, 4u};
}


TEST_CASE("Test magic number") {
  using test_struct_field_list = 
    struct_field_list<
      magic_number<"magic_num", u32, field_size<fixed<4>>, 0xdeadbeef>,
      basic_field<"p", u32, field_size<fixed<4>>>
    >;

  std::ofstream ofs("test_bin_input_magic_number.bin", std::ios::out | std::ios::binary);
  u32 a = 0xdeadbeef;
  u32 b = 0xcafed00d;
  u32 x = 0xbeefbeef;

  ofs.write(reinterpret_cast<const char*>(&a), sizeof(a));
  ofs.write(reinterpret_cast<const char*>(&b), sizeof(b));
  ofs.write(reinterpret_cast<const char*>(&x), sizeof(x));
  ofs.close();

  std::ifstream ifs("test_bin_input_magic_number.bin", std::ios::in | std::ios::binary);
  auto res = struct_cast<test_struct_field_list>(ifs);
  ifs.close();

  REQUIRE(res.has_value());
  auto fields = *res;
  REQUIRE(fields["magic_num"_f] == 0xdeadbeef);
  REQUIRE(fields["p"_f] == 0xcafed00d);
}


TEST_CASE("Test magic array") {
  using test_struct_field_list = 
    struct_field_list<
      magic_byte_array<"magic_arr", 10, std::array<unsigned char, 10>{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}>,
      basic_field<"size", u32, field_size<fixed<4>>>
    >;

  std::ofstream ofs("test_bin_input_magic_array.bin", std::ios::out | std::ios::binary);
  u32 bytes_0_3 = 0xffffffff;
  u32 bytes_4_7 = 0xffffffff;
  u16 bytes_8_9 = 0xffff;
  u32 x = 0xcafed00d;

  ofs.write(reinterpret_cast<const char*>(&bytes_0_3), sizeof(bytes_0_3));
  ofs.write(reinterpret_cast<const char*>(&bytes_4_7), sizeof(bytes_4_7));
  ofs.write(reinterpret_cast<const char*>(&bytes_8_9), sizeof(bytes_8_9));
  ofs.write(reinterpret_cast<const char*>(&x), sizeof(x));
  ofs.close();

  std::ifstream ifs("test_bin_input_magic_array.bin", std::ios::in | std::ios::binary);
  auto res = struct_cast<test_struct_field_list>(ifs);
  ifs.close();

  REQUIRE(res.has_value());
  auto fields = *res;
  REQUIRE(fields["magic_arr"_f] == std::array<u8, 10>{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff});
  REQUIRE(fields["size"_f] == 0xcafed00d);
}


TEST_CASE("Test magic string") {
  using test_struct_field_list = 
    struct_field_list<
      magic_string<"magic_str", "GIF">,
      basic_field<"size", u32, field_size<fixed<4>>>
    >;

  const u8 str[] = "GIF";
  const auto str_len = 3;
  u32 x = 0xcafed00d;

  std::ofstream ofs("test_bin_input_magic_string.bin", std::ios::out | std::ios::binary);
  ofs.write(reinterpret_cast<const char*>(&str), str_len + 1);
  ofs.write(reinterpret_cast<const char*>(&x), sizeof(x));
  ofs.close();

  std::ifstream ifs("test_bin_input_magic_string.bin", std::ios::in | std::ios::binary);
  auto res = struct_cast<test_struct_field_list>(ifs);
  ifs.close();

  REQUIRE(res.has_value());
  auto fields = *res;
  REQUIRE(std::string_view{fields["magic_str"_f].data()} == std::string_view{fixed_string("GIF").data()});
  REQUIRE(fields["size"_f] == 0xcafed00d);
}


TEST_CASE("Test reading a meta_struct with aliased length prefixed string from binary file") {
  using var_buffer_struct = 
    struct_field_list<
      basic_field<"len", std::size_t, field_size<fixed<8>>>,
      str_field<"str", field_size<from_field<"len">>>
    >;

  constexpr std::size_t str_len = 10;
  const u8 str[] = "foo in bar";

  std::ofstream ofs("test_bin_input_str_fields.bin", std::ios::out | std::ios::binary);
  ofs.write(reinterpret_cast<const char*>(&str_len), sizeof(str_len));
  ofs.write(reinterpret_cast<const char*>(&str), str_len);
  ofs.close();

  std::ifstream ifs("test_bin_input_str_fields.bin", std::ios::in | std::ios::binary);
  auto res = struct_cast<var_buffer_struct>(ifs);
  ifs.close();
 
  REQUIRE(res.has_value());
  auto fields = *res;
  REQUIRE(fields["len"_f] == 10);
  REQUIRE(fields["str"_f].size() == 10);
  std::string_view expected{"foo in bar"};
  REQUIRE(std::string_view{fields["str"_f]} == expected);
};


TEST_CASE("Test reading a meta_struct with aliased length prefixed buffer fields from binary file") {
  using var_buffer_struct = 
    struct_field_list<
      basic_field<"len", std::size_t, field_size<fixed<8>>>,
      vec_field<"vec", u32, field_size<from_field<"len">>>
    >;

  constexpr std::size_t vec_len = 10;
  const u8 str[] = "foo in bar";
  const u32 u32_arr[] = {
    0xdeadbeef, 0xcafed00d,
    0xdeadbeef, 0xcafed00d,
    0xdeadbeef, 0xcafed00d,
    0xdeadbeef, 0xcafed00d,
    0xdeadbeef, 0xcafed00d
  };

  std::ofstream ofs("test_bin_input_vec_fields.bin", std::ios::out | std::ios::binary);
  ofs.write(reinterpret_cast<const char*>(&vec_len), sizeof(vec_len));
  ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
  ofs.write(reinterpret_cast<const char*>(&str), vec_len + 1);
  ofs.close();

  std::ifstream ifs("test_bin_input_vec_fields.bin", std::ios::in | std::ios::binary);
  auto res = struct_cast<var_buffer_struct>(ifs);
  ifs.close();
 
  REQUIRE(res.has_value());
  auto fields = *res;
  REQUIRE(fields["len"_f] == 10);
  REQUIRE(fields["vec"_f].size() == 10);
  REQUIRE(fields["vec"_f] == std::vector<u32>{0xdeadbeef, 0xcafed00d, 
                                              0xdeadbeef, 0xcafed00d,
                                              0xdeadbeef, 0xcafed00d,
                                              0xdeadbeef, 0xcafed00d, 
                                              0xdeadbeef, 0xcafed00d});
};


TEST_CASE("Test reading a meta_struct with aliased length prefixed buffer fields depending on multiple fields from binary file") {
  auto size_from_rc = [](auto r, auto c) { return r * c; };

  using var_buffer_struct = 
    struct_field_list<
      basic_field<"row", std::size_t, field_size<fixed<8>>>,
      basic_field<"col", std::size_t, field_size<fixed<8>>>,
      vec_field<"flat_vec", u32, field_size<from_fields<size_from_rc, with_fields<"row", "col">>>>
    >;

  constexpr std::size_t row = 5;
  constexpr std::size_t col = 2;
  const u32 u32_arr[] = {
    0xdeadbeef, 0xcafed00d,
    0xdeadbeef, 0xcafed00d,
    0xdeadbeef, 0xcafed00d,
    0xdeadbeef, 0xcafed00d,
    0xdeadbeef, 0xcafed00d
  };

  std::ofstream ofs("test_bin_input_vec_fields_multiple_size.bin", std::ios::out | std::ios::binary);
  ofs.write(reinterpret_cast<const char*>(&row), sizeof(row));
  ofs.write(reinterpret_cast<const char*>(&col), sizeof(col));
  ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
  ofs.close();

  std::ifstream ifs("test_bin_input_vec_fields_multiple_size.bin", std::ios::in | std::ios::binary);
  auto res = struct_cast<var_buffer_struct>(ifs);
  ifs.close();
 
  REQUIRE(res.has_value());
  auto fields = *res;
  REQUIRE(fields["row"_f] == 5);
  REQUIRE(fields["col"_f] == 2);
  REQUIRE(fields["flat_vec"_f].size() == 10);
  REQUIRE(fields["flat_vec"_f] == std::vector<u32>{0xdeadbeef, 0xcafed00d, 
                                                   0xdeadbeef, 0xcafed00d,
                                                   0xdeadbeef, 0xcafed00d,
                                                   0xdeadbeef, 0xcafed00d, 
                                                   0xdeadbeef, 0xcafed00d});
};


// todo recursive resize
// TEST_CASE("Test reading a meta_struct with aliased length prefixed md_buffer fields depending on multiple fields from binary file") {
//   auto size_from_rc = [](auto r, auto c) { return r * c; };
//
//   using var_buffer_struct = 
//     struct_field_list<
//       basic_field<"row", std::size_t, field_size<fixed<8>>>,
//       basic_field<"col", std::size_t, field_size<fixed<8>>>,
//       vec_field<"matrix", std::vector<u32>, field_size<from_fields<size_from_rc, with_fields<"row", "col">>>>
//     >;
//
//   constexpr std::size_t row = 5;
//   constexpr std::size_t col = 2;
//   const u32 u32_arr[] = {
//     0xdeadbeef, 0xcafed00d,
//     0xdeadbeef, 0xcafed00d,
//     0xdeadbeef, 0xcafed00d,
//     0xdeadbeef, 0xcafed00d,
//     0xdeadbeef, 0xcafed00d
//   };
//
//   std::ofstream ofs("test_bin_input_md_vec_fields.bin", std::ios::out | std::ios::binary);
//   ofs.write(reinterpret_cast<const char*>(&row), sizeof(row));
//   ofs.write(reinterpret_cast<const char*>(&col), sizeof(col));
//   ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
//   ofs.close();
//
//   std::ifstream ifs("test_bin_input_md_vec_fields.bin", std::ios::in | std::ios::binary);
//   // oops resizing recursively is not possible
//   // auto res = struct_cast<var_buffer_struct>(ifs);
//   ifs.close();
//  
//   // REQUIRE(res.has_value());
//   // auto fields = *res;
//   // REQUIRE(fields["row"_f] == 5);
//   // REQUIRE(fields["col"_f] == 2);
//   // REQUIRE(fields["matrix"_f].size() == 10);
//   // REQUIRE(fields["vec"_f] == std::vector<u32>{0xdeadbeef, 0xcafed00d, 
//   //                                             0xdeadbeef, 0xcafed00d,
//   //                                             0xdeadbeef, 0xcafed00d,
//   //                                             0xdeadbeef, 0xcafed00d, 
//   //                                             0xdeadbeef, 0xcafed00d});
// };


TEST_CASE("Dummy test to verify runtime computation from fields") {
  using u32 = unsigned int;
  using sfl = 
    struct_field_list<basic_field<"a", u32, field_size<fixed<4>>>, 
                      basic_field<"b", u32, field_size<fixed<4>>>>;

  std::ofstream ofs("test_bin_input_1.bin", std::ios::out | std::ios::binary);
  u32 a = 4;
  u32 b = 5;
  ofs.write(reinterpret_cast<const char*>(&a), sizeof(a));
  ofs.write(reinterpret_cast<const char*>(&b), sizeof(b));
  ofs.close();

  std::ifstream ifs("test_bin_input_1.bin", std::ios::in | std::ios::binary);
  auto result = struct_cast<sfl>(ifs);
  ifs.close();

  auto callable = [](const u32& a, const u32& b) -> u32 { return a * b; };
  auto fields = *result;
  REQUIRE(fields["a"_f] == 4);
  REQUIRE(fields["b"_f] == 5);
  auto comp_res = compute<callable, u32, with_fields<"a", "b">>{}(fields);
  REQUIRE(comp_res == 20);
}


TEST_CASE("Test case to verify option field parsing with parse predicate failure") {
  auto is_a_eq_1 = [](auto& a){ return a == 1; };

  using test_struct_field_list = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      maybe_field<"c", u32, field_size<fixed<4>>, parse_if<is_a_eq_1, with_fields<"a">>>
    >;

  std::ofstream ofs("test_bin_input_4.bin", std::ios::out | std::ios::binary);
  u32 a = 0xdeadbeef;
  u32 b = 0xcafed00d;
  u32 c = 0xbeefbeef;
  ofs.write(reinterpret_cast<const char*>(&a), sizeof(a));
  ofs.write(reinterpret_cast<const char*>(&b), sizeof(b));
  ofs.write(reinterpret_cast<const char*>(&c), sizeof(c));
  ofs.close();

  std::ifstream ifs("test_bin_input_4.bin", std::ios::in | std::ios::binary);
  auto result = struct_cast<test_struct_field_list>(ifs);
  ifs.close();

  REQUIRE(result.has_value() == true);
  if(result) {
    auto fields = *result;
    REQUIRE(fields["a"_f] == 0xdeadbeef);
    REQUIRE(fields["b"_f] == 0xcafed00d);
    REQUIRE(fields["c"_f] == std::nullopt);
  }
}


TEST_CASE("Test case to verify option field parsing from binary file with successful parse predicate") {
  auto is_a_eq_deadbeef = [](auto a){ return a == 0xdeadbeef; };

  using test_struct_field_list = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      maybe_field<"c", u32, field_size<fixed<4>>, parse_if<is_a_eq_deadbeef, with_fields<"a">>>
    >;

  std::ofstream ofs("test_bin_input_5.bin", std::ios::out | std::ios::binary);
  u32 a = 0xdeadbeef;
  u32 b = 0xcafed00d;
  u32 c = 0xbeefbeef;
  ofs.write(reinterpret_cast<const char*>(&a), sizeof(a));
  ofs.write(reinterpret_cast<const char*>(&b), sizeof(b));
  ofs.write(reinterpret_cast<const char*>(&c), sizeof(c));
  ofs.close();

  std::ifstream ifs("test_bin_input_5.bin", std::ios::in | std::ios::binary);
  auto result = struct_cast<test_struct_field_list>(ifs);
  ifs.close();

  REQUIRE(result.has_value() == true);
  if(result) {
    auto fields = *result;
    REQUIRE(fields["a"_f] == 0xdeadbeef);
    REQUIRE(fields["b"_f] == 0xcafed00d);
    REQUIRE(*(fields["c"_f]) == 0xbeefbeef);
  }
}


TEST_CASE("Test case to verify variant field parsing from a binary file") {
  using test_struct_field_list = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      union_field<
        "c", 
        type<
          match_field<"a">,
          type_switch<
            match_case<0xcafed00d, type_tag<float, field_size<fixed<4>>>>,
            match_case<0xdeadbeef, type_tag<u32, field_size<fixed<4>>>>,
            match_case<0xbeefbeef, type_tag<int, field_size<fixed<4>>>>
          >
        >
      >
    >;

  std::ofstream ofs("test_bin_input_6.bin", std::ios::out | std::ios::binary);
  u32 a = 0xdeadbeef;
  u32 b = 0xcafed00d;
  u32 c = 0xbeefbeef;
  ofs.write(reinterpret_cast<const char*>(&a), sizeof(a));
  ofs.write(reinterpret_cast<const char*>(&b), sizeof(b));
  ofs.write(reinterpret_cast<const char*>(&c), sizeof(c));
  ofs.close();

  std::ifstream ifs("test_bin_input_6.bin", std::ios::in | std::ios::binary);

  auto result = struct_cast<test_struct_field_list>(ifs);
  ifs.close();

  REQUIRE(result.has_value() == true);
  if(result) {
    auto fields = *result;
    REQUIRE(fields["a"_f] == 0xdeadbeef);
    REQUIRE(fields["b"_f] == 0xcafed00d);
    REQUIRE(std::get<u32>(fields["c"_f]) == 0xbeefbeef);
  }
}


TEST_CASE("Test case to verify variant field parsing from a binary file with complex type predicate") {
  auto some_complex_calc = [](auto a, auto b){ return a + b; };

  using test_struct_field_list = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      union_field<
        "c", 
        type<
          compute<some_complex_calc, u32, with_fields<"a", "b">>,
          type_switch<
            match_case<100, type_tag<float, field_size<fixed<4>>>>,
            match_case<200, type_tag<u32, field_size<fixed<4>>>>,
            match_case<300, type_tag<int, field_size<fixed<4>>>>
          >
        >
      >
    >;

  std::ofstream ofs("test_bin_input_7.bin", std::ios::out | std::ios::binary);
  u32 a = 100;
  u32 b = 100;
  u32 c = 0xbeefbeef;
  ofs.write(reinterpret_cast<const char*>(&a), sizeof(a));
  ofs.write(reinterpret_cast<const char*>(&b), sizeof(b));
  ofs.write(reinterpret_cast<const char*>(&c), sizeof(c));
  ofs.close();

  std::ifstream ifs("test_bin_input_7.bin", std::ios::in | std::ios::binary);

  auto result = struct_cast<test_struct_field_list>(ifs);
  ifs.close();

  REQUIRE(result.has_value() == true);
  if(result) {
    auto fields = *result;
    REQUIRE(fields["a"_f] == 100);
    REQUIRE(fields["b"_f] == 100);
    REQUIRE(std::get<u32>(fields["c"_f]) == 0xbeefbeef);
  }
}


// todo clause based union field
