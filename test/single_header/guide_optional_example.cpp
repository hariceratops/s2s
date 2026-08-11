// Backs the maybe/presence-deduction example in UserGuide.md. The region
// between docs-begin and docs-end is compared against that fenced block by the
// doc_examples_match test, so edit them together or the check fails.
// docs-begin
#include "s2s.hpp"

#include <sstream>

using namespace s2s_literals;

using u32 = unsigned int;

// "detail" is on the wire only when "kind" is 1. The predicate reads siblings
// that have already been parsed, which is why it must name them.
constexpr auto kind_is_one = [](auto kind) { return kind == 1u; };

using message =
  s2s::struct_field_list<
    s2s::basic_field<"kind", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::maybe<
      s2s::basic_field<"detail", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::parse_if<kind_is_one, s2s::with_fields<"kind">>
    >
  >;

auto write_and_read(u32 kind, bool with_detail) -> bool {
  message obj{};
  obj["kind"_f] = kind;
  if(with_detail)
    obj["detail"_f] = 0xbeefbeefu;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  const auto written = s2s::struct_write_be<message>(stream, obj);
  if(!written)
    return false;

  const auto back = s2s::struct_cast_be<message>(stream);
  if(!back)
    return false;

  // An absent optional is empty; a present one is dereferenced.
  return with_detail ? *((*back)["detail"_f]) == 0xbeefbeefu : true;
}

auto main() -> int {
  // kind == 1: the predicate says present, and the struct supplies it.
  if(!write_and_read(1u, true))
    return 1;

  // kind == 2: the predicate says absent, and the struct leaves it unset.
  if(!write_and_read(2u, false))
    return 1;

  // Disagreement is caught: the predicate says present, the struct is empty.
  message bad{};
  bad["kind"_f] = 1u;
  std::stringstream discard(std::ios::in | std::ios::out | std::ios::binary);
  const auto rejected = s2s::struct_write_be<message>(discard, bad);

  return !rejected
      && rejected.error().failure_reason == s2s::error_reason::validation_failure
        ? 0 : 1;
}
// docs-end
