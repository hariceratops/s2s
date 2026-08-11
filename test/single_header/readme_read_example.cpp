// Backs the read-direction example in README.md. The region between docs-begin
// and docs-end is compared against that fenced block by the doc_examples_match
// test, so edit them together or the check fails.
// docs-begin
#include "s2s.hpp"

#include <print>
#include <sstream>
#include <string>

using namespace s2s_literals;

// Two members: a length field of size 8 and type std::size_t, and a
// length-prefixed string whose length is derived from the "len" field.
using our_struct =
  s2s::struct_field_list<
    s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
    s2s::str_field<"str", s2s::field_size<s2s::len_from_field<"len">>>
  >;

auto main() -> int {
  // Any input stream works; a std::ifstream over a file is the same call.
  std::stringstream input(
    std::string("\x05\x00\x00\x00\x00\x00\x00\x00" "hello", 13),
    std::ios::in | std::ios::binary);

  const auto res =
    s2s::struct_cast_le<our_struct>(input)
      .transform([](const our_struct& fields){
        std::println("len={} str={}", fields["len"_f], fields["str"_f]);
        return fields;
      }).transform_error([](const s2s::cast_error& err){
        std::println("failure_reason={} failed_at={}",
                     static_cast<int>(err.failure_reason), err.failed_at);
        return err;
      });

  return res ? 0 : 1;
}
// docs-end
