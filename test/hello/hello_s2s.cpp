#include "../../single_header/s2s.hpp"
#include <print>
#include <expected>
#include <fstream>


using namespace s2s_literals;


// Our "struct" has 2 members. A length field of size 8 and type
// std::size_t, and a length prefixed string whose length is 
// derived from the "len" field
using our_struct = 
  s2s::struct_field_list<
    s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
    s2s::str_field<"str", s2s::field_size<s2s::len_from_field<"len">>>
  >;

auto main(void) -> int {
  std::ifstream ifs("sample.bin", std::ios::in | std::ios::binary);
  auto res = 
    s2s::struct_cast_le<our_struct>(ifs)
      .transform([](const our_struct& fields){
        std::println("len={} str={}", fields["len"_f], fields["str"_f]);
        return fields;
      }).transform_error([](const s2s::cast_error& err){
        std::println("failure_reason={} failed_at={}", static_cast<int>(err.failure_reason), err.failed_at);
        return err;
      });

  return 0;
}
