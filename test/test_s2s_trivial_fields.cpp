#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include "../single_header/s2s.hpp"
#include "s2s_test_utils.hpp"


using namespace s2s_literals;


TEST_CASE("Test reading a meta_struct from a binary file") {
  PREPARE_INPUT_FILE({
    u32 a = 0xdeadbeef;
    u32 b = 0xcafed00d;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
  });

  FIELD_LIST_SCHEMA = 
   s2s::struct_field_list<
     s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
     s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>
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
   s2s::struct_field_list<
     s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>, s2s::eq(0xdeadbeef)>, 
     s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>, s2s::eq(0xcafed00d)>
  >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == false);
    auto err = result.error();
    REQUIRE(err.failure_reason == s2s::error_reason::validation_failure);
    REQUIRE(err.failed_at == "b");
  });
}


TEST_CASE("Test reading a meta_struct from a binary when file buffer exhausts") {
  PREPARE_INPUT_FILE({
    u32 a = 0xdeadbeef;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.close();
  });

  FIELD_LIST_SCHEMA = 
   s2s::struct_field_list<
     s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
     s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>
  >;

  FIELD_LIST_LE_READ_CHECK({
    REQUIRE(result.has_value() == false);
    auto err = result.error();
    REQUIRE(err.failure_reason == s2s::error_reason::buffer_exhaustion);
    REQUIRE(err.failed_at == "b");
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
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::struct_field<
        "c", 
        s2s::struct_field_list<
          s2s::basic_field<"x", u32, s2s::field_size<s2s::fixed<4>>>,
          s2s::basic_field<"y", u32, s2s::field_size<s2s::fixed<4>>>
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

