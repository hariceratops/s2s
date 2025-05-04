
#include <gtest/gtest.h>
#include "../single_header/s2s.hpp"
#include "s2s_test_utils.hpp"


using namespace s2s_literals;


TEST(S2STest, successful_parse_predicate) {
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
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::maybe<
        s2s::basic_field<"c", u32, s2s::field_size<s2s::fixed<4>>>, 
        s2s::parse_if<is_a_eq_deadbeef, s2s::with_fields<"a">>
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_EQ(result.has_value(), true);
    if(result) {
      auto fields = *result;
      ASSERT_EQ(fields["a"_f], 0xdeadbeef);
      ASSERT_EQ(fields["b"_f], 0xcafed00d);
      ASSERT_EQ(*(fields["c"_f]), 0xbeefbeef);
    }
  });
}


TEST(S2STest, parse_predicate_failure) {
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
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::maybe<
        s2s::basic_field<"c", u32, s2s::field_size<s2s::fixed<4>>>, 
        s2s::parse_if<is_a_eq_1, s2s::with_fields<"a">>
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_EQ(result.has_value(), true);
    if(result) {
      auto fields = *result;
      ASSERT_EQ(fields["a"_f], 0xdeadbeef);
      ASSERT_EQ(fields["b"_f], 0xcafed00d);
      ASSERT_EQ(fields["c"_f], std::nullopt);
    }
  });
}


TEST(S2STest, optional_struct_successful_parse_predicate) {
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
   s2s::struct_field_list<
     s2s::basic_field<"x", u32, s2s::field_size<s2s::fixed<4>>>, 
     s2s::basic_field<"y", u32, s2s::field_size<s2s::fixed<4>>>
  >;
  FIELD_LIST_SCHEMA = 
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::maybe<
        s2s::struct_field<"c", inner>, 
        s2s::parse_if<is_a_eq_deadbeef, s2s::with_fields<"a">>
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_EQ(result.has_value(), true);
    if(result) {
      auto fields = *result;
      ASSERT_EQ(fields["a"_f], 0xdeadbeef);
      ASSERT_EQ(fields["b"_f], 0xcafed00d);
      auto opt_struct = *(fields["c"_f]);
      ASSERT_EQ(opt_struct["x"_f], 0xbeefbeef);
      ASSERT_EQ(opt_struct["y"_f], 0xbeefd00d);
    }
  });
}


TEST(S2STest, optional_fixed_array_successful_parse_predicate) {
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
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::maybe<
        s2s::fixed_array_field<"c", u32, 3>, 
        s2s::parse_if<is_a_eq_deadbeef, s2s::with_fields<"a">>
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_EQ(result.has_value(), true);
    if(result) {
      auto fields = *result;
      ASSERT_EQ(fields["a"_f], 0xdeadbeef);
      ASSERT_EQ(fields["b"_f], 0xcafed00d);
      ASSERT_TRUE(fields["c"_f]);
      auto opt_array = *(fields["c"_f]);
      ASSERT_EQ(opt_array, (std::array<u32, 3>{0xdeadbeef, 0xcafed00d, 0xbeefbeef}));
    }
  });
}


TEST(S2STest, optional_fixed_string_with_successful_parse_predicate) {
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
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::maybe<
        s2s::fixed_string_field<"c", 10>, 
        s2s::parse_if<is_a_eq_deadbeef, s2s::with_fields<"a">>
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    std::string_view expected{"foo in bar"};
    ASSERT_EQ(result.has_value(), true);
    if(result) {
      auto fields = *result;
      ASSERT_EQ(fields["a"_f], 0xdeadbeef);
      ASSERT_EQ(fields["b"_f], 0xcafed00d);
      ASSERT_TRUE(fields["c"_f]);
      auto opt_fixed_str = *(fields["c"_f]);
      ASSERT_EQ(std::string_view{opt_fixed_str.data()}, expected);
    }
  });
}


TEST(S2STest, optional_length_prefixed_array_with_successful_parse_predicate) {
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
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
      s2s::maybe<
        s2s::vec_field<
          "vec", 
          u32, 
          s2s::field_size<s2s::len_from_field<"len">>
        >, 
        s2s::parse_if<is_a_eq_deadbeef, s2s::with_fields<"a">>
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_EQ(result.has_value(), true);
    if(result) {
      auto fields = *result;
      ASSERT_EQ(fields["a"_f], 0xdeadbeef);
      ASSERT_EQ(fields["b"_f], 0xcafed00d);
      ASSERT_EQ(fields["len"_f], 10);
      ASSERT_TRUE(fields["vec"_f]);
      auto vec = *(fields["vec"_f]);
      ASSERT_EQ(vec.size(), 10);
      ASSERT_EQ(vec, (std::vector<u32>{0xdeadbeef, 0xcafed00d, 
                                      0xdeadbeef, 0xcafed00d,
                                      0xdeadbeef, 0xcafed00d,
                                      0xdeadbeef, 0xcafed00d, 
                                      0xdeadbeef, 0xcafed00d}));
    }
  });
}


TEST(S2STest, optional_array_of_records_with_successful_parse_predicate) {
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
    s2s::struct_field_list <
      s2s::basic_field<"x", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::basic_field<"y", u32, s2s::field_size<s2s::fixed<4>>>
    >;
  FIELD_LIST_SCHEMA = 
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::maybe<
        s2s::array_of_records<"records", test_struct, 3>, 
        s2s::parse_if<is_a_eq_deadbeef, s2s::with_fields<"a">>
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_EQ(result.has_value(), true);
    if(result) {
      auto fields = *result;
      ASSERT_EQ(fields["a"_f], 0xdeadbeef);
      ASSERT_EQ(fields["b"_f], 0xcafed00d);
      ASSERT_TRUE(fields["records"_f]);
      auto records = fields["records"_f];
      for(auto record: *records) {
        ASSERT_EQ(record["x"_f], 0xdeadbeef);
        ASSERT_EQ(record["y"_f], 0xbeefbeef);
      }
    }
  });
}


TEST(S2STest, optional_vector_of_records_with_successful_parse_predicate) {
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
    s2s::struct_field_list <
      s2s::basic_field<"x", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::basic_field<"y", u32, s2s::field_size<s2s::fixed<4>>>
    >;
  FIELD_LIST_SCHEMA = 
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
      s2s::maybe<
        s2s::vector_of_records<
          "records", 
          test_struct, 
          s2s::field_size<s2s::len_from_field<"len">>
        >, 
        s2s::parse_if<is_a_eq_deadbeef, s2s::with_fields<"a">>
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_EQ(result.has_value(), true);
    if(result) {
      auto fields = *result;
      ASSERT_EQ(fields["a"_f], 0xdeadbeef);
      ASSERT_EQ(fields["b"_f], 0xcafed00d);
      ASSERT_TRUE(fields["records"_f]);
      auto records = fields["records"_f];
      for(auto record: *records) {
        ASSERT_EQ(record["x"_f], 0xdeadbeef);
        ASSERT_EQ(record["y"_f], 0xbeefbeef);
      }
    }
  });
}


