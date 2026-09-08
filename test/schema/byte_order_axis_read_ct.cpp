// The byte-order axis, read side — constant-evaluable tier.
// Spec:   dev/specs/byte-order-is-data-not-a-template-parameter.md
// Design: dev/design/byte-order-is-data-not-a-template-parameter.md §10
//
// Includes include/s2s.hpp rather than the amalgam, as every _ct file does.
// That is not incidental here: it is what discharges the design's finding 10 —
// the new public headers must be added to include/s2s.hpp's hand-maintained
// roster, and this file is the only thing in the suite that would notice if
// they were not. The amalgam picks them up automatically and so proves nothing.
//
// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.

#include <array>
#include <ut>

#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

auto main() -> int {
  // TODO(054): the TIFF-shaped schema of design §3.2 read in both the II and
  // MM forms, asserting the u16 magic comes out as 42 both ways — the whole
  // claim of the feature is that the same four bytes decode differently.
  // TODO(054): a marker matching neither form, asserting
  // cast_error{validation_failure, "byte_order"}.
  // TODO(054): a control — a schema with no announcement reads identically
  // under struct_cast_le.
  // TODO(055): the ladder form; a compute_t form over TWO fields of the
  // announcing record (the case a single match_field cannot express, and
  // therefore the one that justifies marking the record rather than the
  // field); and a three-way equivalence over the same bytes.
  // TODO(056): the over-reach guard — an order-dependent LATER sibling of the
  // containing record, asserted to read correctly. This matters more than the
  // negative cases: it proves the check does not extend past its boundary.
  // TODO(056): a magic_byte_array marker and a fixed_string marker are both
  // accepted.
  // TODO(057): announcing record one level deep, then two. Assert on a later
  // sibling OF THE CONTAINER and on a record after the container closes as
  // SEPARATE cases — one test covering both passes with the reference
  // threading half-broken.
  "placeholder — replaced by 054"_test = [] constexpr { expect(eq(1, 1)); };
}
