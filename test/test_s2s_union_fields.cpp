#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include "../single_header/struct_cast.hpp"
#include "s2s_test_utils.hpp"


using namespace s2s_literals;


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
      s2s::union_field<
        "c", 
        s2s::type<
          s2s::match_field<"a">,
          s2s::type_switch<
            s2s::match_case<0xcafed00d, s2s::struct_tag<inner_1>>,
            s2s::match_case<0xdeadbeef, s2s::struct_tag<inner_2>>
          >
        >
      >
    >;


  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == false);
    auto err = result.error();
    REQUIRE(err.failure_reason == s2s::error_reason::type_deduction_failure);
    REQUIRE(err.failed_at == "c");
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
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::union_field<
        "c", 
        s2s::type<
          s2s::match_field<"a">,
          s2s::type_switch<
            s2s::match_case<
              0xcafed00d, 
              s2s::trivial_tag<float, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::match_case<
              0xdeadbeef, 
              s2s::trivial_tag<u32, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::match_case<
              0xbeefbeef, 
              s2s::trivial_tag<int, s2s::field_size<s2s::fixed<4>>>
            >
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


TEST_CASE("Test case to verify variant field with an s2s::fixed array parsing from a binary file") {
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
      s2s::union_field<
        "c", 
        s2s::type<
          s2s::match_field<"a">,
          s2s::type_switch<
            s2s::match_case<
              0xcafed00d, 
              s2s::trivial_tag<float, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::match_case<
              0xdeadbeef, 
              s2s::fixed_buffer_tag<u32, 3>
            >,
            s2s::match_case<
              0xbeefbeef, 
              s2s::trivial_tag<int, s2s::field_size<s2s::fixed<4>>>
            >
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
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
      s2s::union_field<
        "c", 
        s2s::type<
          s2s::match_field<"a">,
          s2s::type_switch<
            s2s::match_case<
              0xcafed00d, 
              s2s::trivial_tag<float, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::match_case<
              0xdeadbeef, 
              s2s::variable_buffer_tag<u32, s2s::field_size<s2s::len_from_field<"len">>>
            >,
            s2s::match_case<
              0xbeefbeef, 
              s2s::trivial_tag<int, s2s::field_size<s2s::fixed<4>>>
            >
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


TEST_CASE("Test case to verify variant field with an s2s::fixed string parsing from a binary file") {
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
      s2s::union_field<
        "c", 
        s2s::type<
          s2s::match_field<"a">,
          s2s::type_switch<
            s2s::match_case<
              0xcafed00d, 
              s2s::trivial_tag<float, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::match_case<
              0xdeadbeef, s2s::fixed_string_tag<10>
            >,
            s2s::match_case<
              0xbeefbeef, 
              s2s::trivial_tag<int, s2s::field_size<s2s::fixed<4>>>
            >
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
      auto& str = std::get<s2s::fixed_string<10>>(fields["c"_f]);
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
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
      s2s::union_field<
        "c", 
        s2s::type<
          s2s::match_field<"a">,
          s2s::type_switch<
            s2s::match_case<
              0xcafed00d, 
              s2s::trivial_tag<float, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::match_case<
              0xdeadbeef, 
              s2s::variable_string_tag<s2s::field_size<s2s::len_from_field<"len">>>
            >,
            s2s::match_case<
              0xbeefbeef, 
              s2s::trivial_tag<int, s2s::field_size<s2s::fixed<4>>>
            >
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
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::union_field<
        "c", 
        s2s::type<
          s2s::compute<some_complex_calc, u32, s2s::with_fields<"a", "b">>,
          s2s::type_switch<
            s2s::match_case<
              100, 
              s2s::trivial_tag<float, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::match_case<
              200, 
              s2s::trivial_tag<u32, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::match_case<
              300, 
              s2s::trivial_tag<int, s2s::field_size<s2s::fixed<4>>>
            >
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
      s2s::union_field<
        "c", 
        s2s::type<
          s2s::match_field<"a">,
          s2s::type_switch<
            s2s::match_case<0xcafed00d, s2s::struct_tag<inner_1>>,
            s2s::match_case<0xdeadbeef, s2s::struct_tag<inner_2>>
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
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::union_field<
        "c", 
        s2s::type<
          s2s::type_ladder<
            s2s::clause<
              s2s::predicate<bpred_1, s2s::with_fields<"a", "b">>, 
              s2s::trivial_tag<float, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::clause<
              s2s::predicate<bpred_2, s2s::with_fields<"a", "b">>, 
              s2s::trivial_tag<u32, s2s::field_size<s2s::fixed<4>>>
            >
          >
        >
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == false);
    auto err = result.error();
    REQUIRE(err.failure_reason == s2s::error_reason::type_deduction_failure);
    REQUIRE(err.failed_at == "c");
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
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::union_field<
        "c", 
        s2s::type<
          s2s::type_ladder<
            s2s::clause<
              s2s::predicate<bpred_1, s2s::with_fields<"a", "b">>, 
              s2s::trivial_tag<float, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::clause<
              s2s::predicate<bpred_2, s2s::with_fields<"a", "b">>, 
              s2s::trivial_tag<u32, s2s::field_size<s2s::fixed<4>>>
            >,
            s2s::clause<
              s2s::predicate<bpred_3, s2s::with_fields<"a", "b">>, 
              s2s::trivial_tag<int, s2s::field_size<s2s::fixed<4>>>
            >
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

