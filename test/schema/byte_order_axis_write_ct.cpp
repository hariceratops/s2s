// The byte-order axis, write side — constant-evaluable tier.
// Spec:   dev/specs/byte-order-is-data-not-a-template-parameter.md
// Design: dev/design/byte-order-is-data-not-a-template-parameter.md §10
//
// Kept apart from byte_order_axis_read_ct rather than merged into one
// round-trip, per the tree's convention and because a round-trip stays green
// if both directions are wrong the same way — which, for a byte-order feature,
// is the likely failure.
//
// No `using namespace ut;` — see the read-side file.

#include <array>
#include <ut>

#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

auto main() -> int {
  // TODO(059): byte-for-byte round trip through struct_cast and stream_cast in
  // both the II and MM forms.
  // TODO(059): set fl["byte_order"_f]["marker"_f] directly and observe the
  // output bytes change — this is the criterion that the marker keeps its
  // setter and is not a frozen field.
  // TODO(059): the nested case. Write-side resolution at depth is a separate
  // code path from read.
  // TODO(059): the frozen composition — an eq-constrained marker, asserting
  // the write succeeds from a DEFAULT-CONSTRUCTED struct. This is precisely
  // what fails without design §7.2's seeding, so this test is the proof of
  // that decision, not a nicety.
  "placeholder — replaced by 059"_test = [] constexpr { expect(eq(1, 1)); };
}
