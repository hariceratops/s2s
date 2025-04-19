#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include "../single_header/struct_cast.hpp"
#include <cstdlib>
#include <iostream>
#include <cassert>


#define PREPARE_INPUT_FILE(code) \
  []() { \
    std::ofstream file("test_input.bin", std::ios::out | std::ios::binary); \
    code; \
  }()


#define FIELD_LIST_SCHEMA using test_field_list

#define FIELD_LIST_LE_READ_CHECK(code) \
  do { \
    std::ifstream file("test_input.bin", std::ios::in | std::ios::binary); \
    auto result = struct_cast_le<test_field_list>(file); \
    code; \
  } while(0)

#define FIELD_LIST_BE_READ_CHECK(code) \
  do { \
    std::ifstream file("test_input.bin", std::ios::in | std::ios::binary); \
    auto result = struct_cast_be<test_field_list>(file); \
    code; \
  } while(0)


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
// using temp = 
//   struct_field_list<
//     basic_field<"a", int, field_size<fixed<4>>>,
//     basic_field<"b", int, field_size<fixed<4>>>
//   >;
// will fail
// using non_unique_temp = 
//   struct_field_list<
//     basic_field<"a", int, field_size<fixed<4>>>,
//     basic_field<"a", int, field_size<fixed<4>>>
//   >;
// using u32 = unsigned int;
// static_assert(array_of_records_like<std::array<temp, 10>>);
// static_assert(vector_of_records_like<std::vector<temp>>);
// static_assert(!field_containable<std::array<temp, 10>>);
// will fail since type choices are not unique
// using test_struct_field_list = 
//     struct_field_list<
//       basic_field<"a", u32, field_size<fixed<4>>>, 
//       basic_field<"b", u32, field_size<fixed<4>>>,
//       union_field<
//         "c", 
//         type<
//           match_field<"a">,
//           type_switch<
//             match_case<0xcafed00d, type_tag<float, field_size<fixed<4>>>>,
//             match_case<0xdeadbeef, type_tag<int, field_size<fixed<4>>>>,
//             match_case<0xbeefbeef, type_tag<int, field_size<fixed<4>>>>
//           >
//         >
//       >
//     >;
//
// will fail!
// using size_failure_basic_field = basic_field<"a", int, field_size<fixed<6>>>;

// Helper types
using i32 = int;
using u32 = unsigned int;
using u8 = unsigned char;
using u16 = unsigned short;


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


// TEST_CASE("Test open range interval check") {
//   // constexpr auto ranges = std::array<range<u32>, 3>{range{2u, 4u}, range{6u, 9u}, range{13u, 15u}};
//   // constexpr auto r = range{2u, 4u};
// }


TEST_CASE("Test reading a meta_struct from a binary file") {
  PREPARE_INPUT_FILE({
    u32 a = 0xdeadbeef;
    u32 b = 0xcafed00d;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
  });

  FIELD_LIST_SCHEMA = 
   struct_field_list<
     basic_field<"a", u32, field_size<fixed<4>>>, 
     basic_field<"b", u32, field_size<fixed<4>>>
  >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == true);
    if(result) {
      auto fields = *result;
      REQUIRE(fields["a"_f] == 0xdeadbeef);
      REQUIRE(fields["b"_f] == 0xcafed00d);
    }
  });

  FIELD_LIST_BE_READ_CHECK({
    REQUIRE(result.has_value() == true);
    if(result) {
      auto fields = *result;
      REQUIRE(fields["a"_f] == 0xefbeadde);
      REQUIRE(fields["b"_f] == 0x0dd0feca);
    }
  });
}


TEST_CASE("Test reading a meta_struct from a binary file but validation of field value fails") {
  PREPARE_INPUT_FILE({
    u32 a = 0xdeadbeef;
    u32 b = 0xdeadbeef;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
  });

  FIELD_LIST_SCHEMA = 
   struct_field_list<
     basic_field<"a", u32, field_size<fixed<4>>, eq(0xdeadbeef)>, 
     basic_field<"b", u32, field_size<fixed<4>>, eq(0xcafed00d)>
  >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == false);
    REQUIRE(result.error() == cast_error::validation_failure);
  });
}


TEST_CASE("Test reading a meta_struct from a binary when file buffer exhausts") {
  PREPARE_INPUT_FILE({
    u32 a = 0xdeadbeef;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.close();
  });

  FIELD_LIST_SCHEMA = 
   struct_field_list<
     basic_field<"a", u32, field_size<fixed<4>>>, 
     basic_field<"b", u32, field_size<fixed<4>>>
  >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == false);
    REQUIRE(result.error() == cast_error::buffer_exhaustion);
  });
}


TEST_CASE("Test reading a meta_struct with nested struct from a binary file") {
  PREPARE_INPUT_FILE({
    u32 a = 0xdeadbeef;
    u32 b = 0xcafed00d;
    u32 x = 0xbeefbeef;
    u32 y = 0xdeadbeef;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.write(reinterpret_cast<const char*>(&x), sizeof(x));
    file.write(reinterpret_cast<const char*>(&y), sizeof(y));
  });

  FIELD_LIST_SCHEMA = 
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

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == true);
    if(result) {
      auto fields = *result;
      REQUIRE(fields["a"_f] == 0xdeadbeef);
      REQUIRE(fields["b"_f] == 0xcafed00d);
      REQUIRE(fields["c"_f]["x"_f] == 0xbeefbeef);
      REQUIRE(fields["c"_f]["y"_f] == 0xdeadbeef);
    }
  });
}


TEST_CASE("Test reading a meta_struct with fixed buffer fields from binary file") {
  [](){
    std::ofstream file("test_input.bin", std::ios::out | std::ios::binary); \
    constexpr std::size_t str_len = 10;
    const u8 str[] = "foo in bar";
    const u32 u32_arr[] = {0xdeadbeef, 0xcafed00d, 0xbeefbeef};
    file.write(reinterpret_cast<const char*>(&str), str_len + 1);
    file.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
  }();

  FIELD_LIST_SCHEMA = 
    struct_field_list<
      // c_str_field<"a", 10>,
      fixed_string_field<"b", 10>,
      fixed_array_field<"c", u32, 3>
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


TEST_CASE("Test reading a meta_struct with multidimensional fixed buffer field from binary file") {
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
    struct_field_list<
      fixed_array_field<"arr", std::array<u32, 3>, 3>
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


// todo array of records and vector of records
TEST_CASE("Test reading a meta_struct with array of records from binary file") {
  []() {
    std::ofstream ofs("test_input.bin", std::ios::out | std::ios::binary);
    const u32 u32_arr[3][2] = { 
      {0xdeadbeef, 0xbeefbeef},
      {0xdeadbeef, 0xbeefbeef}, 
      {0xdeadbeef, 0xbeefbeef} 
    };
    ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
    ofs.close();
  }();

  using test_struct = 
    struct_field_list <
      basic_field<"a", u32, field_size<fixed<4>>>,
      basic_field<"b", u32, field_size<fixed<4>>>
    >;
  FIELD_LIST_SCHEMA = 
    struct_field_list<
      array_of_records<"records", test_struct, 3>
    >;
  
  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value());
    auto fields = *result;
    auto records = fields["records"_f];
    for(auto record: records) {
      REQUIRE(record["a"_f] == 0xdeadbeef);
      REQUIRE(record["b"_f] == 0xbeefbeef);
    }
  });
};


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
    struct_field_list<
      magic_number<"magic_num", u32, field_size<fixed<4>>, 0xdeadbeef>,
      basic_field<"p", u32, field_size<fixed<4>>>
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
    struct_field_list<
      magic_byte_array<"magic_arr", 10, std::array<unsigned char, 10>{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}>,
      basic_field<"size", u32, field_size<fixed<4>>>
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
    struct_field_list<
      magic_string<"magic_str", "GIF">,
      basic_field<"size", u32, field_size<fixed<4>>>
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value());
    auto fields = *result;
    REQUIRE(std::string_view{fields["magic_str"_f].data()} == std::string_view{fixed_string("GIF").data()});
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
    struct_field_list<
      magic_string<"magic_str", "GIF">,
      basic_field<"size", u32, field_size<fixed<4>>>
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(!result.has_value());
    REQUIRE(result.error() == cast_error::validation_failure);
  });
}


TEST_CASE("Test reading a meta_struct with aliased length prefixed string from binary file") {
  PREPARE_INPUT_FILE({
    constexpr std::size_t str_len = 10;
    const u8 str[] = "foo in bar";
    file.write(reinterpret_cast<const char*>(&str_len), sizeof(str_len));
    file.write(reinterpret_cast<const char*>(&str), str_len);
  });

  FIELD_LIST_SCHEMA = 
    struct_field_list<
      basic_field<"len", std::size_t, field_size<fixed<8>>>,
      str_field<"str", field_size<len_from_field<"len">>>
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value());
    auto fields = *result;
    REQUIRE(fields["len"_f] == 10);
    REQUIRE(fields["str"_f].size() == 10);
    std::string_view expected{"foo in bar"};
    REQUIRE(std::string_view{fields["str"_f]} == expected);
  });
};


TEST_CASE("Test reading a meta_struct with aliased length prefixed buffer fields from binary file") {
  []() {
    constexpr std::size_t vec_len = 10;
    const u8 str[] = "foo in bar";
    const u32 u32_arr[] = {
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d
    };
    std::ofstream file("test_input.bin", std::ios::out | std::ios::binary);
    file.write(reinterpret_cast<const char*>(&vec_len), sizeof(vec_len));
    file.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
    file.write(reinterpret_cast<const char*>(&str), vec_len + 1);
  }();

  FIELD_LIST_SCHEMA = 
    struct_field_list<
      basic_field<"len", std::size_t, field_size<fixed<8>>>,
      vec_field<"vec", u32, field_size<len_from_field<"len">>>
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value());
    auto fields = *result;
    REQUIRE(fields["len"_f] == 10);
    REQUIRE(fields["vec"_f].size() == 10);
    REQUIRE(fields["vec"_f] == std::vector<u32>{0xdeadbeef, 0xcafed00d, 
                                                0xdeadbeef, 0xcafed00d,
                                                0xdeadbeef, 0xcafed00d,
                                                0xdeadbeef, 0xcafed00d, 
                                                0xdeadbeef, 0xcafed00d});
  });
};


TEST_CASE("Test reading a meta_struct with aliased length prefixed buffer fields depending on multiple fields from binary file") {
  []() {
    constexpr std::size_t row = 5;
    constexpr std::size_t col = 2;
    const u32 u32_arr[] = {
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d
    };
    std::ofstream file("test_input.bin", std::ios::out | std::ios::binary);
    file.write(reinterpret_cast<const char*>(&row), sizeof(row));
    file.write(reinterpret_cast<const char*>(&col), sizeof(col));
    file.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
    file.close();
  }();

  auto size_from_rc = [](auto r, auto c) { return r * c; };
  FIELD_LIST_SCHEMA = 
    struct_field_list<
      basic_field<"row", std::size_t, field_size<fixed<8>>>,
      basic_field<"col", std::size_t, field_size<fixed<8>>>,
      vec_field<"flat_vec", u32, field_size<len_from_fields<size_from_rc, with_fields<"row", "col">>>>
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value());
    auto fields = *result;
    REQUIRE(fields["row"_f] == 5);
    REQUIRE(fields["col"_f] == 2);
    REQUIRE(fields["flat_vec"_f].size() == 10);
    REQUIRE(fields["flat_vec"_f] == std::vector<u32>{0xdeadbeef, 0xcafed00d, 
                                                     0xdeadbeef, 0xcafed00d,
                                                     0xdeadbeef, 0xcafed00d,
                                                     0xdeadbeef, 0xcafed00d, 
                                                     0xdeadbeef, 0xcafed00d});
  });
};


TEST_CASE("Test reading a meta_struct with vector of records from binary file") {
  []() {
    constexpr std::size_t vec_len = 3;
    const u32 u32_arr[3][2] = { 
      {0xdeadbeef, 0xbeefbeef},
      {0xdeadbeef, 0xbeefbeef}, 
      {0xdeadbeef, 0xbeefbeef} 
    };
    std::ofstream file("test_input.bin", std::ios::out | std::ios::binary);
    file.write(reinterpret_cast<const char*>(&vec_len), sizeof(vec_len));
    file.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
    file.close();
  }();

  using test_struct = 
    struct_field_list <
      basic_field<"a", u32, field_size<fixed<4>>>,
      basic_field<"b", u32, field_size<fixed<4>>>
    >;
  FIELD_LIST_SCHEMA = 
    struct_field_list<
      basic_field<"len", std::size_t, field_size<fixed<8>>>,
      vector_of_records<"records", test_struct, field_size<len_from_field<"len">>>
    >;
 
  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value());
    auto fields = *result;
    auto records = fields["records"_f];
    for(auto record: records) {
      REQUIRE(record["a"_f] == 0xdeadbeef);
      REQUIRE(record["b"_f] == 0xbeefbeef);
    }
  });
};


// // todo recursive resize
// // TEST_CASE("Test reading a meta_struct with aliased length prefixed md_buffer fields depending on multiple fields from binary file") {
// //   auto size_from_rc = [](auto r, auto c) { return r * c; };
// //
// //   using var_buffer_struct = 
// //     struct_field_list<
// //       basic_field<"row", std::size_t, field_size<fixed<8>>>,
// //       basic_field<"col", std::size_t, field_size<fixed<8>>>,
// //       vec_field<"matrix", std::vector<u32>, field_size<from_fields<size_from_rc, with_fields<"row", "col">>>>
// //     >;
// //
// //   constexpr std::size_t row = 5;
// //   constexpr std::size_t col = 2;
// //   const u32 u32_arr[] = {
// //     0xdeadbeef, 0xcafed00d,
// //     0xdeadbeef, 0xcafed00d,
// //     0xdeadbeef, 0xcafed00d,
// //     0xdeadbeef, 0xcafed00d,
// //     0xdeadbeef, 0xcafed00d
// //   };
// //
// //   std::ofstream ofs("test_input_md_vec_fields.bin", std::ios::out | std::ios::binary);
// //   ofs.write(reinterpret_cast<const char*>(&row), sizeof(row));
// //   ofs.write(reinterpret_cast<const char*>(&col), sizeof(col));
// //   ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
// //   ofs.close();
// //
// //   std::ifstream ifs("test_input_md_vec_fields.bin", std::ios::in | std::ios::binary);
// //   // oops resizing recursively is not possible
// //   // auto res = struct_cast<var_buffer_struct>(ifs);
// //   ifs.close();
// //  
// //   // REQUIRE(res.has_value());
// //   // auto fields = *res;
// //   // REQUIRE(fields["row"_f] == 5);
// //   // REQUIRE(fields["col"_f] == 2);
// //   // REQUIRE(fields["matrix"_f].size() == 10);
// //   // REQUIRE(fields["vec"_f] == std::vector<u32>{0xdeadbeef, 0xcafed00d, 
// //   //                                             0xdeadbeef, 0xcafed00d,
// //   //                                             0xdeadbeef, 0xcafed00d,
// //   //                                             0xdeadbeef, 0xcafed00d, 
// //   //                                             0xdeadbeef, 0xcafed00d});
// // };
//
//
TEST_CASE("Dummy test to verify runtime computation from fields") {
  PREPARE_INPUT_FILE({
    u32 a = 4;
    u32 b = 5;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.close();
  });

  FIELD_LIST_SCHEMA = 
    struct_field_list<basic_field<"a", u32, field_size<fixed<4>>>, 
                      basic_field<"b", u32, field_size<fixed<4>>>>;

  // todo: macro fails to compile
  [](){
    std::ifstream ifs("test_input.bin", std::ios::in | std::ios::binary);
    auto result = struct_cast_le<test_field_list>(ifs);
    auto callable = [](const u32& a, const u32& b) -> u32 { return a * b; };
    auto fields = *result;
    REQUIRE(fields["a"_f] == 4);
    REQUIRE(fields["b"_f] == 5);
    auto comp_res = compute<callable, u32, with_fields<"a", "b">>{}(fields);
    REQUIRE(comp_res == 20);
  }();
}


TEST_CASE("Test case to verify option field parsing from binary file with successful parse predicate") {
  PREPARE_INPUT_FILE({
    u32 a = 0xdeadbeef;
    u32 b = 0xcafed00d;
    u32 c = 0xbeefbeef;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.write(reinterpret_cast<const char*>(&c), sizeof(c));
  });

  auto is_a_eq_deadbeef = [](auto a){ return a == 0xdeadbeef; };
  FIELD_LIST_SCHEMA = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      maybe_field<basic_field<"c", u32, field_size<fixed<4>>>, parse_if<is_a_eq_deadbeef, with_fields<"a">>>
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == true);
    if(result) {
      auto fields = *result;
      REQUIRE(fields["a"_f] == 0xdeadbeef);
      REQUIRE(fields["b"_f] == 0xcafed00d);
      REQUIRE(*(fields["c"_f]) == 0xbeefbeef);
    }
  });
}


TEST_CASE("Test case to verify option field parsing with parse predicate failure") {
  PREPARE_INPUT_FILE({
    u32 a = 0xdeadbeef;
    u32 b = 0xcafed00d;
    u32 c = 0xbeefbeef;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.write(reinterpret_cast<const char*>(&c), sizeof(c));
  });

  auto is_a_eq_1 = [](auto& a){ return a == 1; };
  FIELD_LIST_SCHEMA = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      maybe_field<basic_field<"c", u32, field_size<fixed<4>>>, parse_if<is_a_eq_1, with_fields<"a">>>
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == true);
    if(result) {
      auto fields = *result;
      REQUIRE(fields["a"_f] == 0xdeadbeef);
      REQUIRE(fields["b"_f] == 0xcafed00d);
      REQUIRE(fields["c"_f] == std::nullopt);
    }
  });
}


TEST_CASE("Test case to verify optional struct from binary file with successful parse predicate") {
  PREPARE_INPUT_FILE({
    u32 a = 0xdeadbeef;
    u32 b = 0xcafed00d;
    u32 c = 0xbeefbeef;
    u32 d = 0xbeefd00d;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.write(reinterpret_cast<const char*>(&c), sizeof(c));
    file.write(reinterpret_cast<const char*>(&d), sizeof(d));
  });

  auto is_a_eq_deadbeef = [](auto a){ return a == 0xdeadbeef; };
  using inner = 
   struct_field_list<
     basic_field<"x", u32, field_size<fixed<4>>>, 
     basic_field<"y", u32, field_size<fixed<4>>>
  >;
  FIELD_LIST_SCHEMA = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      maybe_field<struct_field<"c", inner>, parse_if<is_a_eq_deadbeef, with_fields<"a">>>
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == true);
    if(result) {
      auto fields = *result;
      REQUIRE(fields["a"_f] == 0xdeadbeef);
      REQUIRE(fields["b"_f] == 0xcafed00d);
      auto opt_struct = *(fields["c"_f]);
      REQUIRE(opt_struct["x"_f] == 0xbeefbeef);
      REQUIRE(opt_struct["y"_f] == 0xbeefd00d);
    }
  });
}


TEST_CASE("Test case to verify optional fixed_array from binary file with successful parse predicate") {
  [](){
    std::ofstream file("test_input.bin", std::ios::out | std::ios::binary);
    u32 a = 0xdeadbeef;
    u32 b = 0xcafed00d;
    const u32 u32_arr[] = {0xdeadbeef, 0xcafed00d, 0xbeefbeef};
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
  }();

  auto is_a_eq_deadbeef = [](auto a){ return a == 0xdeadbeef; };
  FIELD_LIST_SCHEMA = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      maybe_field<fixed_array_field<"c", u32, 3>, parse_if<is_a_eq_deadbeef, with_fields<"a">>>
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == true);
    if(result) {
      auto fields = *result;
      REQUIRE(fields["a"_f] == 0xdeadbeef);
      REQUIRE(fields["b"_f] == 0xcafed00d);
      REQUIRE(fields["c"_f]);
      auto opt_array = *(fields["c"_f]);
      REQUIRE(opt_array == std::array<u32, 3>{0xdeadbeef, 0xcafed00d, 0xbeefbeef});
    }
  });
}


TEST_CASE("Test case to verify optional fixed string from binary file with successful parse predicate") {
  PREPARE_INPUT_FILE({
    u32 a = 0xdeadbeef;
    u32 b = 0xcafed00d;
    constexpr std::size_t str_len = 10;
    const u8 str[] = "foo in bar";
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.write(reinterpret_cast<const char*>(&str), str_len + 1);
  });

  auto is_a_eq_deadbeef = [](auto a){ return a == 0xdeadbeef; };
  FIELD_LIST_SCHEMA = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      maybe_field<fixed_string_field<"c", 10>, parse_if<is_a_eq_deadbeef, with_fields<"a">>>
    >;

  FIELD_LIST_LE_READ_CHECK({
    std::string_view expected{"foo in bar"};
    REQUIRE(result.has_value() == true);
    if(result) {
      auto fields = *result;
      REQUIRE(fields["a"_f] == 0xdeadbeef);
      REQUIRE(fields["b"_f] == 0xcafed00d);
      REQUIRE(fields["c"_f]);
      auto opt_fixed_str = *(fields["c"_f]);
      REQUIRE(std::string_view{opt_fixed_str.data()} == expected);
    }
  });
}


TEST_CASE("Test case to verify optional length prefixed array from binary file with successful parse predicate") {
  [](){
    std::ofstream file("test_input.bin", std::ios::out | std::ios::binary);
    u32 a = 0xdeadbeef;
    u32 b = 0xcafed00d;
    constexpr std::size_t vec_len = 10;
    const u32 u32_arr[] = {
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d
    };
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.write(reinterpret_cast<const char*>(&vec_len), sizeof(vec_len));
    file.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
  }();

  auto is_a_eq_deadbeef = [](auto a){ return a == 0xdeadbeef; };
  FIELD_LIST_SCHEMA = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      basic_field<"len", std::size_t, field_size<fixed<8>>>,
      maybe_field<vec_field<"vec", u32, field_size<len_from_field<"len">>>, parse_if<is_a_eq_deadbeef, with_fields<"a">>>
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == true);
    if(result) {
      auto fields = *result;
      REQUIRE(fields["a"_f] == 0xdeadbeef);
      REQUIRE(fields["b"_f] == 0xcafed00d);
      REQUIRE(fields["len"_f] == 10);
      REQUIRE(fields["vec"_f]);
      auto vec = *(fields["vec"_f]);
      REQUIRE(vec.size() == 10);
      REQUIRE(vec == std::vector<u32>{0xdeadbeef, 0xcafed00d, 
                                      0xdeadbeef, 0xcafed00d,
                                      0xdeadbeef, 0xcafed00d,
                                      0xdeadbeef, 0xcafed00d, 
                                      0xdeadbeef, 0xcafed00d});
    }
  });
}


TEST_CASE("Test case to verify optional array of records from binary file with successful parse predicate") {
  [](){
    std::ofstream file("test_input.bin", std::ios::out | std::ios::binary);
    u32 a = 0xdeadbeef;
    u32 b = 0xcafed00d;
    const u32 u32_arr[3][2] = { 
      {0xdeadbeef, 0xbeefbeef},
      {0xdeadbeef, 0xbeefbeef}, 
      {0xdeadbeef, 0xbeefbeef} 
    };
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
  }();

  auto is_a_eq_deadbeef = [](auto a){ return a == 0xdeadbeef; };
  using test_struct = 
    struct_field_list <
      basic_field<"x", u32, field_size<fixed<4>>>,
      basic_field<"y", u32, field_size<fixed<4>>>
    >;
  FIELD_LIST_SCHEMA = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      maybe_field<array_of_records<"records", test_struct, 3>, parse_if<is_a_eq_deadbeef, with_fields<"a">>>
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == true);
    if(result) {
      auto fields = *result;
      REQUIRE(fields["a"_f] == 0xdeadbeef);
      REQUIRE(fields["b"_f] == 0xcafed00d);
      REQUIRE(fields["records"_f]);
      auto records = fields["records"_f];
      for(auto record: *records) {
        REQUIRE(record["x"_f] == 0xdeadbeef);
        REQUIRE(record["y"_f] == 0xbeefbeef);
      }
    }
  });
}


TEST_CASE("Test case to verify optional vector of records from binary file with successful parse predicate") {
  []() {
    std::ofstream file("test_input.bin", std::ios::out | std::ios::binary);
    u32 a = 0xdeadbeef;
    u32 b = 0xcafed00d;
    constexpr std::size_t vec_len = 3;
    const u32 u32_arr[3][2] = { 
      {0xdeadbeef, 0xbeefbeef},
      {0xdeadbeef, 0xbeefbeef}, 
      {0xdeadbeef, 0xbeefbeef} 
    };
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.write(reinterpret_cast<const char*>(&vec_len), sizeof(vec_len));
    file.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
  }();

  auto is_a_eq_deadbeef = [](auto a){ return a == 0xdeadbeef; };
  using test_struct = 
    struct_field_list <
      basic_field<"x", u32, field_size<fixed<4>>>,
      basic_field<"y", u32, field_size<fixed<4>>>
    >;
  FIELD_LIST_SCHEMA = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      basic_field<"len", std::size_t, field_size<fixed<8>>>,
      maybe_field<vector_of_records<"records", test_struct, field_size<len_from_field<"len">>>, parse_if<is_a_eq_deadbeef, with_fields<"a">>>
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == true);
    if(result) {
      auto fields = *result;
      REQUIRE(fields["a"_f] == 0xdeadbeef);
      REQUIRE(fields["b"_f] == 0xcafed00d);
      REQUIRE(fields["records"_f]);
      auto records = fields["records"_f];
      for(auto record: *records) {
        REQUIRE(record["x"_f] == 0xdeadbeef);
        REQUIRE(record["y"_f] == 0xbeefbeef);
      }
    }
  });
}


TEST_CASE("Test case to verify failed parsing variant field") {
  PREPARE_INPUT_FILE({
    u32 a = 0xbeefbeef;
    u32 b = 0xbeefbeef;
    u32 c = 0xbeefbeef;
    u32 d = 0xbeefd00d;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.write(reinterpret_cast<const char*>(&c), sizeof(c));
    file.write(reinterpret_cast<const char*>(&d), sizeof(d));
  });

  using inner_1 = 
   struct_field_list<
     basic_field<"x", u32, field_size<fixed<4>>>, 
     basic_field<"y", u32, field_size<fixed<4>>>
  >;
  using inner_2 = 
   struct_field_list<
     basic_field<"p", u32, field_size<fixed<4>>>, 
     basic_field<"q", u32, field_size<fixed<4>>>
  >;
  FIELD_LIST_SCHEMA = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      union_field<
        "c", 
        type<
          match_field<"a">,
          type_switch<
            match_case<0xcafed00d, struct_tag<inner_1>>,
            match_case<0xdeadbeef, struct_tag<inner_2>>
          >
        >
      >
    >;


  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == false);
    REQUIRE(result.error() == cast_error::type_deduction_failure);
  });
}


TEST_CASE("Test case to verify variant field parsing from a binary file") {
  PREPARE_INPUT_FILE({
    u32 a = 0xdeadbeef;
    u32 b = 0xcafed00d;
    u32 c = 0xbeefbeef;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.write(reinterpret_cast<const char*>(&c), sizeof(c));
  });

  FIELD_LIST_SCHEMA = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      union_field<
        "c", 
        type<
          match_field<"a">,
          type_switch<
            match_case<0xcafed00d, trivial_tag<float, field_size<fixed<4>>>>,
            match_case<0xdeadbeef, trivial_tag<u32, field_size<fixed<4>>>>,
            match_case<0xbeefbeef, trivial_tag<int, field_size<fixed<4>>>>
          >
        >
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == true);
    if(result) {
      auto fields = *result;
      REQUIRE(fields["a"_f] == 0xdeadbeef);
      REQUIRE(fields["b"_f] == 0xcafed00d);
      REQUIRE(std::get<u32>(fields["c"_f]) == 0xbeefbeef);
    }
  });
}


TEST_CASE("Test case to verify variant field with an fixed array parsing from a binary file") {
  [](){
    std::ofstream file("test_input.bin", std::ios::out | std::ios::binary);
    u32 a = 0xdeadbeef;
    u32 b = 0xcafed00d;
    const u32 u32_arr[] = {0xdeadbeef, 0xcafed00d, 0xbeefbeef};
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
  }();

  FIELD_LIST_SCHEMA = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      union_field<
        "c", 
        type<
          match_field<"a">,
          type_switch<
            match_case<0xcafed00d, trivial_tag<float, field_size<fixed<4>>>>,
            match_case<0xdeadbeef, fixed_buffer_tag<u32, 3>>,
            match_case<0xbeefbeef, trivial_tag<int, field_size<fixed<4>>>>
          >
        >
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == true);
    if(result) {
      auto fields = *result;
      REQUIRE(fields["a"_f] == 0xdeadbeef);
      REQUIRE(fields["b"_f] == 0xcafed00d);
      REQUIRE(std::get<std::array<u32, 3>>(fields["c"_f]) == std::array<u32, 3>{0xdeadbeef, 0xcafed00d, 0xbeefbeef});
    }
  });
}


TEST_CASE("Test case to verify variant field with an variable sized array parsing from a binary file") {
  [](){
    std::ofstream file("test_input.bin", std::ios::out | std::ios::binary);
    u32 a = 0xdeadbeef;
    u32 b = 0xcafed00d;
    constexpr std::size_t vec_len = 10;
    const u32 u32_arr[] = {
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d
    };
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.write(reinterpret_cast<const char*>(&vec_len), sizeof(vec_len));
    file.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
  }();

  FIELD_LIST_SCHEMA = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      basic_field<"len", std::size_t, field_size<fixed<8>>>,
      union_field<
        "c", 
        type<
          match_field<"a">,
          type_switch<
            match_case<0xcafed00d, trivial_tag<float, field_size<fixed<4>>>>,
            match_case<0xdeadbeef, variable_buffer_tag<u32, field_size<len_from_field<"len">>>>,
            match_case<0xbeefbeef, trivial_tag<int, field_size<fixed<4>>>>
          >
        >
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == true);
    if(result) {
      auto fields = *result;
      REQUIRE(fields["a"_f] == 0xdeadbeef);
      REQUIRE(fields["b"_f] == 0xcafed00d);
      REQUIRE(fields["len"_f] == 10);
      auto& vec = std::get<std::vector<u32>>(fields["c"_f]);
      REQUIRE(vec.size() == 10);
      REQUIRE(vec == std::vector<u32>{0xdeadbeef, 0xcafed00d, 
                                      0xdeadbeef, 0xcafed00d,
                                      0xdeadbeef, 0xcafed00d,
                                      0xdeadbeef, 0xcafed00d, 
                                      0xdeadbeef, 0xcafed00d});
    }
  });
}


TEST_CASE("Test case to verify variant field with an fixed string parsing from a binary file") {
  PREPARE_INPUT_FILE({
    u32 a = 0xdeadbeef;
    u32 b = 0xcafed00d;
    constexpr std::size_t str_len = 10;
    const u8 str[] = "foo in bar";
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.write(reinterpret_cast<const char*>(&str), str_len + 1);
  });

  FIELD_LIST_SCHEMA = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      union_field<
        "c", 
        type<
          match_field<"a">,
          type_switch<
            match_case<0xcafed00d, trivial_tag<float, field_size<fixed<4>>>>,
            match_case<0xdeadbeef, fixed_string_tag<10>>,
            match_case<0xbeefbeef, trivial_tag<int, field_size<fixed<4>>>>
          >
        >
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == true);
    if(result) {
      auto fields = *result;
      REQUIRE(fields["a"_f] == 0xdeadbeef);
      REQUIRE(fields["b"_f] == 0xcafed00d);
      auto& str = std::get<fixed_string<10>>(fields["c"_f]);
      REQUIRE(std::string_view{str.data()} == std::string_view{"foo in bar"});
    }
  });
}


TEST_CASE("Test case to verify variant field with a variable string parsing from a binary file") {
  PREPARE_INPUT_FILE({
    u32 a = 0xdeadbeef;
    u32 b = 0xcafed00d;
    constexpr std::size_t str_len = 10;
    const u8 str[] = "foo in bar";
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.write(reinterpret_cast<const char*>(&str_len), sizeof(str_len));
    file.write(reinterpret_cast<const char*>(&str), str_len + 1);
  });

  FIELD_LIST_SCHEMA = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      basic_field<"len", std::size_t, field_size<fixed<8>>>,
      union_field<
        "c", 
        type<
          match_field<"a">,
          type_switch<
            match_case<0xcafed00d, trivial_tag<float, field_size<fixed<4>>>>,
            match_case<0xdeadbeef, variable_string_tag<field_size<len_from_field<"len">>>>,
            match_case<0xbeefbeef, trivial_tag<int, field_size<fixed<4>>>>
          >
        >
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == true);
    if(result) {
      auto fields = *result;
      REQUIRE(fields["a"_f] == 0xdeadbeef);
      REQUIRE(fields["b"_f] == 0xcafed00d);
      auto& str = std::get<std::string>(fields["c"_f]);
      REQUIRE(str == std::string{"foo in bar"});
    }
  });
}


TEST_CASE("Test case to verify variant field parsing from a binary file with complex type predicate") {
  PREPARE_INPUT_FILE({
    u32 a = 100;
    u32 b = 100;
    u32 c = 0xbeefbeef;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.write(reinterpret_cast<const char*>(&c), sizeof(c));
  });

  auto some_complex_calc = [](auto a, auto b){ return a + b; };
  FIELD_LIST_SCHEMA = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      union_field<
        "c", 
        type<
          compute<some_complex_calc, u32, with_fields<"a", "b">>,
          type_switch<
            match_case<100, trivial_tag<float, field_size<fixed<4>>>>,
            match_case<200, trivial_tag<u32, field_size<fixed<4>>>>,
            match_case<300, trivial_tag<int, field_size<fixed<4>>>>
          >
        >
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == true);
    if(result) {
      auto fields = *result;
      REQUIRE(fields["a"_f] == 100);
      REQUIRE(fields["b"_f] == 100);
      REQUIRE(std::get<u32>(fields["c"_f]) == 0xbeefbeef);
    }
  });
}


TEST_CASE("Test case to verify parsing variant field with multiple struct field choices") {
  PREPARE_INPUT_FILE({
    u32 a = 0xdeadbeef;
    u32 b = 0xcafed00d;
    u32 c = 0xbeefbeef;
    u32 d = 0xbeefd00d;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.write(reinterpret_cast<const char*>(&c), sizeof(c));
    file.write(reinterpret_cast<const char*>(&d), sizeof(d));
  });

  using inner_1 = 
   struct_field_list<
     basic_field<"x", u32, field_size<fixed<4>>>, 
     basic_field<"y", u32, field_size<fixed<4>>>
  >;
  using inner_2 = 
   struct_field_list<
     basic_field<"p", u32, field_size<fixed<4>>>, 
     basic_field<"q", u32, field_size<fixed<4>>>
  >;
  FIELD_LIST_SCHEMA = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      union_field<
        "c", 
        type<
          match_field<"a">,
          type_switch<
            match_case<0xcafed00d, struct_tag<inner_1>>,
            match_case<0xdeadbeef, struct_tag<inner_2>>
          >
        >
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == true);
    if(result) {
      auto fields = *result;
      REQUIRE(fields["a"_f] == 0xdeadbeef);
      REQUIRE(fields["b"_f] == 0xcafed00d);
      auto c = std::get<inner_2>(fields["c"_f]);
      REQUIRE(c["p"_f] == 0xbeefbeef);
      REQUIRE(c["q"_f] == 0xbeefd00d);
    }
  });
}


TEST_CASE("Test case to verify failed variant field parsing from a binary file with boolean clauses") {
  PREPARE_INPUT_FILE({
    u32 a = 50000;
    u32 b = 50000;
    float c = 3.14f;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.write(reinterpret_cast<const char*>(&c), sizeof(c));
  });

  auto bpred_1 = [](auto a, auto b){ return a + b >= 20000 && a + b < 40000; };
  auto bpred_2 = [](auto a, auto b){ return a + b <= 40000 && a + b < 60000; };
  
  // todo type tag entries shall be unique with respect to type
  // todo possible convinent short hand for eval_bool_from_fields
  FIELD_LIST_SCHEMA = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      union_field<
        "c", 
        type<
          type_ladder<
            clause<predicate<bpred_1, with_fields<"a", "b">>, trivial_tag<float, field_size<fixed<4>>>>,
            clause<predicate<bpred_2, with_fields<"a", "b">>, trivial_tag<u32, field_size<fixed<4>>>>
          >
        >
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == false);
    REQUIRE(result.error() == cast_error::type_deduction_failure);
  });
}


TEST_CASE("Test case to verify variant field parsing from a binary file with boolean clauses") {
  PREPARE_INPUT_FILE({
    u32 a = 12000;
    u32 b = 12000;
    float c = 3.14f;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.write(reinterpret_cast<const char*>(&c), sizeof(c));
  });

  auto bpred_1 = [](auto a, auto b){ return a + b >= 20000 && a + b < 40000; };
  auto bpred_2 = [](auto a, auto b){ return a + b <= 40000 && a + b < 60000; };
  auto bpred_3 = [](auto a, auto b){ return a + b >= 60000; };
  
  // todo type tag entries shall be unique with respect to type
  // todo possible convinent short hand for eval_bool_from_fields
  FIELD_LIST_SCHEMA = 
    struct_field_list<
      basic_field<"a", u32, field_size<fixed<4>>>, 
      basic_field<"b", u32, field_size<fixed<4>>>,
      union_field<
        "c", 
        type<
          type_ladder<
            clause<predicate<bpred_1, with_fields<"a", "b">>, trivial_tag<float, field_size<fixed<4>>>>,
            clause<predicate<bpred_2, with_fields<"a", "b">>, trivial_tag<u32, field_size<fixed<4>>>>,
            clause<predicate<bpred_3, with_fields<"a", "b">>, trivial_tag<int, field_size<fixed<4>>>>
          >
        >
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == true);
    if(result) {
      auto fields = *result;
      REQUIRE(fields["a"_f] == 12000);
      REQUIRE(fields["b"_f] == 12000);
      REQUIRE(std::get<float>(fields["c"_f]) == 3.14f);
    }
  });
}

