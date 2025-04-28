#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include "../single_header/s2s.hpp"
#include "s2s_test_utils.hpp"


using namespace s2s_literals;


TEST_CASE("Test to verify runtime computation from fields") {
  PREPARE_INPUT_FILE({
    u32 a = 4;
    u32 b = 5;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.close();
  });

  FIELD_LIST_SCHEMA = 
    s2s::struct_field_list<s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
                      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>>;

  // todo: macro fails to compile
  [](){
    std::ifstream ifs("test_input.bin", std::ios::in | std::ios::binary);
    auto result = s2s::struct_cast_le<test_field_list>(ifs);
    auto callable = [](const u32& a, const u32& b) -> u32 { return a * b; };
    auto fields = *result;
    REQUIRE(fields["a"_f] == 4);
    REQUIRE(fields["b"_f] == 5);
    auto comp_res = s2s::compute<callable, u32, s2s::with_fields<"a", "b">>{}(fields);
    REQUIRE(comp_res == 20);
  }();
}

