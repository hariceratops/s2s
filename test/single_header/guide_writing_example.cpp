// Backs the worked example in UserGuide.md's Writing section. The region
// between docs-begin and docs-end is compared against that fenced block by the
// doc_examples_match test, so edit them together or the check fails.
//
// doc_example.cpp is the other half of this pair: it verifies the prose claims
// the Writing section makes about derived fields, which are asserted rather
// than shown and so are not part of any fenced block.
// docs-begin
#include "s2s.hpp"
#include <sstream>
#include <vector>

using namespace s2s_literals;
using u16 = unsigned short;
using u32 = unsigned int;

using our_struct =
  s2s::struct_field_list<
    s2s::magic_string<"magic", "S2S">,
    s2s::basic_field<"count", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::vec_field<"data", u16, s2s::field_size<s2s::len_from_field<"count">>>
  >;

auto main() -> int {
  our_struct obj{};
  obj["magic"_f] = s2s::fixed_string<3>("S2S");
  obj["data"_f] = std::vector<u16>{0x1122, 0x3344};
  // Note: "count" is never assigned. It is derived from data.size().

  std::stringstream le(std::ios::in | std::ios::out | std::ios::binary);
  if(auto res = s2s::struct_write_le<our_struct>(le, obj); !res)
    return 1;

  std::stringstream be(std::ios::in | std::ios::out | std::ios::binary);
  if(auto res = s2s::struct_write_be<our_struct>(be, obj); !res)
    return 1;

  // Read either one back with the matching byte order.
  auto back = s2s::struct_cast_be<our_struct>(be);
  return back && (*back)["count"_f] == 2 ? 0 : 1;
}
// docs-end
