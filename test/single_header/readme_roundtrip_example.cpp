// Backs the write-direction example in README.md. The region between the
// docs-begin and docs-end markers is compared against that fenced block by the
// doc_examples_match test, so edit them together or the check fails.
// docs-begin
#include "s2s.hpp"

#include <sstream>
#include <vector>

using namespace s2s_literals;

using u16 = unsigned short;
using u32 = unsigned int;

// One schema, both directions.
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
  // "count" is never assigned. It is derived from data.size() during the write.

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  if(const auto written = s2s::struct_write_be<our_struct>(stream, obj); !written)
    return 1;

  const auto back = s2s::struct_cast_be<our_struct>(stream);
  if(!back)
    return 1;

  return (*back)["count"_f] == 2
      && (*back)["data"_f] == std::vector<u16>{0x1122, 0x3344} ? 0 : 1;
}
// docs-end
