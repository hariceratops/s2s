// The announcement census — the load-bearing metaprogram behind 056, 058
// and 060.
// Design: dev/design/byte-order-is-data-not-a-template-parameter.md §10
//
// Lives here rather than under schema/ because it asserts a type-level
// computation, not a round-trip — the same reason union_choice_pipeline_ct
// does.
//
// Why it exists at all: every criterion in 056, 058 and 060 is negative
// ("fails to compile"), and a negative cannot distinguish a census that
// counted correctly from one that counted wrongly in a way nothing observes
// yet. This file is the positive form. Written and passing before 054, so it
// is a description first and a regression gate afterwards.
//
// Verify it is non-vacuous by flipping one expected count and watching it fail.
//
// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture.

#include <ut>

#include "../../include/s2s.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

auto main() -> int {
  // TODO(056): static_asserts over census_of_fields<...>::value for:
  //   - a flat schema with no announcement            -> 0
  //   - one at the top                                -> 1
  //   - one nested two records deep                   -> 1
  //   - one behind a maybe                            -> off_spine == 1
  //   - one inside an array_of_records                -> off_spine == 1
  //   - one inside a union alternative                -> off_spine == 1
  //   - two in disjoint subtrees                      -> 2
  // TODO(056): the order-agnostic predicate asserted directly over each of the
  // value-type rows in design §4.1's table.
  "placeholder — replaced by 056"_test = [] constexpr { expect(eq(1, 1)); };
}
