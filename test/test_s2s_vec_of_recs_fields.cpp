#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include "../single_header/struct_cast.hpp"
#include "s2s_test_utils.hpp"


using namespace s2s_literals;


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
    s2s::struct_field_list <
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>
    >;
  FIELD_LIST_SCHEMA = 
    s2s::struct_field_list<
      s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
      s2s::vector_of_records<
        "records", 
        test_struct, 
        s2s::field_size<s2s::len_from_field<"len">>
      >
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

