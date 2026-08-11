// Backs the constraints example in UserGuide.md. The region between docs-begin
// and docs-end is compared against that fenced block by the doc_examples_match
// test, so edit them together or the check fails.
// docs-begin
#include "s2s.hpp"

#include <sstream>

using namespace s2s_literals;

using u16 = unsigned short;
using u32 = unsigned int;

using record =
  s2s::struct_field_list<
    s2s::magic_string<"magic", "S2S">,
    s2s::basic_field<"version", u16, s2s::field_size<s2s::fixed<2>>, s2s::any_of{u16{1}, u16{2}}>,
    s2s::basic_field<"count", u32, s2s::field_size<s2s::fixed<4>>, s2s::gte{u32{1}}>
  >;

auto main() -> int {
  record ok{};
  ok["magic"_f] = s2s::fixed_string<3>("S2S");
  ok["version"_f] = u16{2};
  ok["count"_f] = 7u;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  if(const auto written = s2s::struct_write_be<record>(stream, ok); !written)
    return 1;
  if(!s2s::struct_cast_be<record>(stream))
    return 1;

  // Writing: the constraint is checked before the field's first byte is out.
  record bad_version = ok;
  bad_version["version"_f] = u16{9};
  std::stringstream discard(std::ios::in | std::ios::out | std::ios::binary);
  const auto rejected = s2s::struct_write_be<record>(discard, bad_version);
  if(!(!rejected
       && rejected.error().failure_reason == s2s::error_reason::validation_failure
       && rejected.error().failed_at == std::string_view{"version"}))
    return 1;

  // Reading: the same constraint rejects the same value off the wire.
  record bad_count = ok;
  bad_count["count"_f] = 0u;
  std::stringstream on_wire(std::ios::in | std::ios::out | std::ios::binary);
  // Write it through a schema with no constraint so the bytes actually exist.
  using unchecked =
    s2s::struct_field_list<
      s2s::magic_string<"magic", "S2S">,
      s2s::basic_field<"version", u16, s2s::field_size<s2s::fixed<2>>>,
      s2s::basic_field<"count", u32, s2s::field_size<s2s::fixed<4>>>
    >;
  unchecked loose{};
  loose["magic"_f] = s2s::fixed_string<3>("S2S");
  loose["version"_f] = u16{2};
  loose["count"_f] = 0u;
  if(const auto written = s2s::struct_write_be<unchecked>(on_wire, loose); !written)
    return 1;

  const auto read_back = s2s::struct_cast_be<record>(on_wire);
  return !read_back
      && read_back.error().failure_reason == s2s::error_reason::validation_failure
      && read_back.error().failed_at == std::string_view{"count"}
        ? 0 : 1;
}
// docs-end
