// Backs the four-axes example in UserGuide.md's Schema section. The region
// between docs-begin and docs-end is compared against that fenced block by the
// doc_examples_match test, so edit them together or the check fails.
// docs-begin
#include "s2s.hpp"

#include <sstream>
#include <vector>

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

using packet =
  s2s::struct_field_list<
    // id         type    size                              constraint
    s2s::magic_string<"magic", "PKT">,
    s2s::basic_field<"version", u16, s2s::field_size<s2s::fixed<2>>, s2s::any_of{u16{1}, u16{2}}>,
    s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::vec_field<"payload", u8, s2s::field_size<s2s::len_from_field<"len">>>
  >;

auto main() -> int {
  packet obj{};
  obj["magic"_f] = s2s::fixed_string<3>("PKT");
  obj["version"_f] = u16{2};
  obj["payload"_f] = std::vector<u8>{0xde, 0xad, 0xbe, 0xef};

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  if(const auto written = s2s::struct_write_be<packet>(stream, obj); !written)
    return 1;

  const auto back = s2s::struct_cast_be<packet>(stream);
  if(!back)
    return 1;

  // "len" was never assigned; it is the size axis of "payload" resolved.
  return (*back)["len"_f] == 4 && (*back)["version"_f] == 2 ? 0 : 1;
}
// docs-end
