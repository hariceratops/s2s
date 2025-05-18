
#include <gtest/gtest.h>
#include "../single_header/s2s.hpp"
#include "s2s_test_utils.hpp"


using namespace s2s_literals;


TEST(S2STest, failed_parsing_variant_field) {
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
   s2s::struct_field_list<
     s2s::basic_field<"x", u32, s2s::field_size<s2s::fixed<4>>>, 
     s2s::basic_field<"y", u32, s2s::field_size<s2s::fixed<4>>>
  >;
  using inner_2 = 
   s2s::struct_field_list<
     s2s::basic_field<"p", u32, s2s::field_size<s2s::fixed<4>>>, 
     s2s::basic_field<"q", u32, s2s::field_size<s2s::fixed<4>>>
  >;
  FIELD_LIST_SCHEMA = 
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::variance<
        "c", 
        s2s::type<
          s2s::match_field<"a">,
          s2s::type_switch<
            s2s::match_case<0xcafed00d, s2s::as_struct<inner_1>>,
            s2s::match_case<0xdeadbeef, s2s::as_struct<inner_2>>
          >
        >
      >
    >;


  FIELD_LIST_LE_READ_CHECK({
    ASSERT_EQ(result.has_value(), false);
    auto err = result.error();
    ASSERT_EQ(err.failure_reason, s2s::error_reason::type_deduction_failure);
    ASSERT_EQ(err.failed_at, "c");
  });
}


TEST(S2STest, successful_variant_field_parsing) {
  PREPARE_INPUT_FILE({
    u32 a = 0xdeadbeef;
    u32 b = 0xcafed00d;
    u32 c = 0xbeefbeef;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.write(reinterpret_cast<const char*>(&c), sizeof(c));
  });

  FIELD_LIST_SCHEMA = 
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::variance<
        "c", 
        s2s::type<
          s2s::match_field<"a">,
          s2s::type_switch<
            s2s::match_case<
              0xcafed00d, 
              s2s::as_trivial<float, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::match_case<
              0xdeadbeef, 
              s2s::as_trivial<u32, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::match_case<
              0xbeefbeef, 
              s2s::as_trivial<int, s2s::field_size<s2s::fixed<4>>>
            >
          >
        >
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_EQ(result.has_value(), true);
    if(result) {
      auto fields = *result;
      ASSERT_EQ(fields["a"_f], 0xdeadbeef);
      ASSERT_EQ(fields["b"_f], 0xcafed00d);
      ASSERT_EQ(std::get<u32>(fields["c"_f]), 0xbeefbeef);
    }
  });
}


TEST(S2STest, variant_field_with_fixed_array) {
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
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::variance<
        "c", 
        s2s::type<
          s2s::match_field<"a">,
          s2s::type_switch<
            s2s::match_case<
              0xcafed00d, 
              s2s::as_trivial<float, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::match_case<
              0xdeadbeef, 
              s2s::as_fixed_arr<u32, 3>
            >,
            s2s::match_case<
              0xbeefbeef, 
              s2s::as_trivial<int, s2s::field_size<s2s::fixed<4>>>
            >
          >
        >
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_EQ(result.has_value(), true);
    if(result) {
      auto fields = *result;
      ASSERT_EQ(fields["a"_f], 0xdeadbeef);
      ASSERT_EQ(fields["b"_f], 0xcafed00d);
      auto c = (std::get<std::array<u32, 3>>(fields["c"_f]));
      ASSERT_EQ(c, (std::array<u32, 3>{0xdeadbeef, 0xcafed00d, 0xbeefbeef}));
    }
  });
}


TEST(S2STest, variant_field_with_variable_sized_array) {
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
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
      s2s::variance<
        "c", 
        s2s::type<
          s2s::match_field<"a">,
          s2s::type_switch<
            s2s::match_case<
              0xcafed00d, 
              s2s::as_trivial<float, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::match_case<
              0xdeadbeef, 
              s2s::as_vec<u32, s2s::field_size<s2s::len_from_field<"len">>>
            >,
            s2s::match_case<
              0xbeefbeef, 
              s2s::as_trivial<int, s2s::field_size<s2s::fixed<4>>>
            >
          >
        >
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_EQ(result.has_value(), true);
    if(result) {
      auto fields = *result;
      ASSERT_EQ(fields["a"_f], 0xdeadbeef);
      ASSERT_EQ(fields["b"_f], 0xcafed00d);
      ASSERT_EQ(fields["len"_f], 10);
      auto& vec = std::get<std::vector<u32>>(fields["c"_f]);
      ASSERT_EQ(vec.size(), 10);
      ASSERT_EQ(vec, (std::vector<u32>{0xdeadbeef, 0xcafed00d, 
                                      0xdeadbeef, 0xcafed00d,
                                      0xdeadbeef, 0xcafed00d,
                                      0xdeadbeef, 0xcafed00d, 
                                      0xdeadbeef, 0xcafed00d}));
    }
  });
}


TEST(S2STest, variant_field_with_fixed_string) {
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
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::variance<
        "c", 
        s2s::type<
          s2s::match_field<"a">,
          s2s::type_switch<
            s2s::match_case<
              0xcafed00d, 
              s2s::as_trivial<float, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::match_case<
              0xdeadbeef, s2s::as_fixed_string<10>
            >,
            s2s::match_case<
              0xbeefbeef, 
              s2s::as_trivial<int, s2s::field_size<s2s::fixed<4>>>
            >
          >
        >
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_EQ(result.has_value(), true);
    if(result) {
      auto fields = *result;
      ASSERT_EQ(fields["a"_f], 0xdeadbeef);
      ASSERT_EQ(fields["b"_f], 0xcafed00d);
      auto& str = std::get<s2s::fixed_string<10>>(fields["c"_f]);
      ASSERT_EQ(std::string_view{str.data()}, std::string_view{"foo in bar"});
    }
  });
}


TEST(S2STest, variant_field_with_variable_string) {
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
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
      s2s::variance<
        "c", 
        s2s::type<
          s2s::match_field<"a">,
          s2s::type_switch<
            s2s::match_case<
              0xcafed00d, 
              s2s::as_trivial<float, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::match_case<
              0xdeadbeef, 
              s2s::as_string<s2s::field_size<s2s::len_from_field<"len">>>
            >,
            s2s::match_case<
              0xbeefbeef, 
              s2s::as_trivial<int, s2s::field_size<s2s::fixed<4>>>
            >
          >
        >
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_EQ(result.has_value(), true);
    if(result) {
      auto fields = *result;
      ASSERT_EQ(fields["a"_f], 0xdeadbeef);
      ASSERT_EQ(fields["b"_f], 0xcafed00d);
      auto& str = std::get<std::string>(fields["c"_f]);
      ASSERT_EQ(str, std::string{"foo in bar"});
    }
  });
}


TEST(S2STest, variant_field_parsing_with_complex_type_predicate) {
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
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::variance<
        "c", 
        s2s::type<
          s2s::compute<some_complex_calc, u32, s2s::with_fields<"a", "b">>,
          s2s::type_switch<
            s2s::match_case<
              100, 
              s2s::as_trivial<float, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::match_case<
              200, 
              s2s::as_trivial<u32, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::match_case<
              300, 
              s2s::as_trivial<int, s2s::field_size<s2s::fixed<4>>>
            >
          >
        >
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_EQ(result.has_value(), true);
    if(result) {
      auto fields = *result;
      ASSERT_EQ(fields["a"_f], 100);
      ASSERT_EQ(fields["b"_f], 100);
      ASSERT_EQ(std::get<u32>(fields["c"_f]), 0xbeefbeef);
    }
  });
}


TEST(S2STest, variant_field_struct_fields) {
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
   s2s::struct_field_list<
     s2s::basic_field<"x", u32, s2s::field_size<s2s::fixed<4>>>, 
     s2s::basic_field<"y", u32, s2s::field_size<s2s::fixed<4>>>
  >;
  using inner_2 = 
   s2s::struct_field_list<
     s2s::basic_field<"p", u32, s2s::field_size<s2s::fixed<4>>>, 
     s2s::basic_field<"q", u32, s2s::field_size<s2s::fixed<4>>>
  >;
  FIELD_LIST_SCHEMA = 
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::variance<
        "c", 
        s2s::type<
          s2s::match_field<"a">,
          s2s::type_switch<
            s2s::match_case<0xcafed00d, s2s::as_struct<inner_1>>,
            s2s::match_case<0xdeadbeef, s2s::as_struct<inner_2>>
          >
        >
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_EQ(result.has_value(), true);
    if(result) {
      auto fields = *result;
      ASSERT_EQ(fields["a"_f], 0xdeadbeef);
      ASSERT_EQ(fields["b"_f], 0xcafed00d);
      auto c = std::get<inner_2>(fields["c"_f]);
      ASSERT_EQ(c["p"_f], 0xbeefbeef);
      ASSERT_EQ(c["q"_f], 0xbeefd00d);
    }
  });
}


TEST(S2STest, variant_field_parsing_with_boolean_clauses) {
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
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::variance<
        "c", 
        s2s::type<
          s2s::type_if_else<
            s2s::branch<
              s2s::predicate<bpred_1, s2s::with_fields<"a", "b">>, 
              s2s::as_trivial<float, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::branch<
              s2s::predicate<bpred_2, s2s::with_fields<"a", "b">>, 
              s2s::as_trivial<u32, s2s::field_size<s2s::fixed<4>>>
            >
          >
        >
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_EQ(result.has_value(), false);
    auto err = result.error();
    ASSERT_EQ(err.failure_reason, s2s::error_reason::type_deduction_failure);
    ASSERT_EQ(err.failed_at, "c");
  });
}


TEST(S2STest, variant_field_with_boolean_clauses) {
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
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::variance<
        "c", 
        s2s::type<
          s2s::type_if_else<
            s2s::branch<
              s2s::predicate<bpred_1, s2s::with_fields<"a", "b">>, 
              s2s::as_trivial<float, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::branch<
              s2s::predicate<bpred_2, s2s::with_fields<"a", "b">>, 
              s2s::as_trivial<u32, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::branch<
              s2s::predicate<bpred_3, s2s::with_fields<"a", "b">>, 
              s2s::as_trivial<int, s2s::field_size<s2s::fixed<4>>>
            >
          >
        >
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_EQ(result.has_value(), true);
    if(result) {
      auto fields = *result;
      ASSERT_EQ(fields["a"_f], 12000);
      ASSERT_EQ(fields["b"_f], 12000);
      ASSERT_EQ(std::get<float>(fields["c"_f]), 3.14f);
    }
  });
}

