// Backs the computed-values example in UserGuide.md. The region between
// docs-begin and docs-end is compared against that fenced block by the
// doc_examples_match test, so edit them together or the check fails.
// docs-begin
#include "s2s.hpp"

#include <sstream>
#include <vector>

using namespace s2s_literals;

using u16 = unsigned short;
using u32 = unsigned int;

// The callable is a non-type template argument, so it has to be usable in a
// constant expression and named somewhere the schema can reach.
constexpr auto area_of = [](auto rows, auto cols) { return rows * cols; };

using matrix =
  s2s::struct_field_list<
    s2s::basic_field<"rows", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"cols", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::vec_field<
      "cells",
      u16,
      s2s::field_size<s2s::len_from_fields<area_of, s2s::with_fields<"rows", "cols">>>
    >
  >;

auto main() -> int {
  matrix obj{};
  // "rows" and "cols" stay assignable: area_of cannot be run backwards, so
  // the library verifies them against cells.size() rather than deriving them.
  obj["rows"_f] = 2u;
  obj["cols"_f] = 3u;
  obj["cells"_f] = std::vector<u16>{1, 2, 3, 4, 5, 6};

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  if(const auto written = s2s::struct_write_be<matrix>(stream, obj); !written)
    return 1;

  const auto back = s2s::struct_cast_be<matrix>(stream);
  if(!back)
    return 1;

  // A disagreement is caught rather than silently written.
  matrix wrong = obj;
  wrong["cols"_f] = 4u;
  std::stringstream discard(std::ios::in | std::ios::out | std::ios::binary);
  const auto rejected = s2s::struct_write_be<matrix>(discard, wrong);

  return (*back)["cells"_f].size() == 6
      && !rejected
      && rejected.error().failure_reason == s2s::error_reason::found_contradicting_length
        ? 0 : 1;
}
// docs-end
