// Backs the worked example in UserGuide.md's Reading section. The region
// between docs-begin and docs-end is compared against that fenced block by the
// doc_examples_match test, so edit them together or the check fails.
// docs-begin
#include "s2s.hpp"

#include <sstream>
#include <string>

using namespace s2s_literals;

using u16 = unsigned short;
using u32 = unsigned int;

using header =
  s2s::struct_field_list<
    s2s::magic_string<"magic", "S2S">,
    s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::str_field<"name", s2s::field_size<s2s::len_from_field<"len">>>,
    s2s::basic_field<"flags", u16, s2s::field_size<s2s::fixed<2>>>
  >;

// The bytes a big-endian "header" occupies, written out by hand:
//   53 32 53 00     magic, "S2S" plus its terminator
//   00 00 00 05     len = 5
//   68 65 6c 6c 6f  name, "hello", exactly len bytes
//   00 01           flags = 1
constexpr auto on_the_wire =
  "\x53\x32\x53\x00"
  "\x00\x00\x00\x05"
  "hello"
  "\x00\x01";

auto main() -> int {
  std::stringstream stream(std::string(on_the_wire, 15),
                           std::ios::in | std::ios::out | std::ios::binary);

  const auto parsed = s2s::struct_cast_be<header>(stream);
  if(!parsed)
    return 1;

  const auto& h = *parsed;
  if(!(h["len"_f] == 5 && h["name"_f] == "hello" && h["flags"_f] == 1))
    return 1;

  // A stream that stops short reports where it ran out, not merely that it did.
  std::stringstream truncated(std::string(on_the_wire, 9),
                              std::ios::in | std::ios::out | std::ios::binary);
  const auto failed = s2s::struct_cast_be<header>(truncated);

  return !failed
      && failed.error().failure_reason == s2s::error_reason::buffer_exhaustion
      && failed.error().failed_at == std::string_view{"name"}
        ? 0 : 1;
}
// docs-end
