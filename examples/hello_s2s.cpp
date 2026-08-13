// A complete s2s program: read a length-prefixed string out of a binary file.
// Built against the shipped single header, the way a consumer would.

#include "s2s.hpp"

#include <expected>
#include <fstream>
#include <print>

using namespace s2s_literals;

// Two fields: a length, and a string whose byte count is derived from it.
using message =
  s2s::struct_field_list<
    s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
    s2s::str_field<"text", s2s::field_size<s2s::len_from_field<"len">>>
  >;

auto main() -> int {
  std::ifstream file("sample.bin", std::ios::in | std::ios::binary);
  if(!file) {
    std::println(stderr, "cannot open sample.bin");
    return 1;
  }

  auto result = s2s::struct_cast_le<message>(file);
  if(!result) {
    std::println(stderr, "read failed: reason={} at={}",
                 static_cast<int>(result.error().failure_reason),
                 result.error().failed_at);
    return 1;
  }

  const auto fields = *result;
  std::println("len={} text={}", fields["text"_f].size(), fields["text"_f]);
  return 0;
}
