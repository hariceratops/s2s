// The trailing option pack rejects what it cannot classify.
//
// 045 makes a field's trailing parameters an order-independent pack of auto
// NTTPs, each classified as a size or a constraint by a named concept. Two
// things must be rejected, and they are rejected by different mechanisms:
// an unrecognised entry fails the per-element placeholder constraint, while a
// duplicate of a kind already supplied fails the design's one sanctioned
// static_assert.
//
// Assert only that the build fails. The design measured gcc 14's wording for
// the per-element form; clang and MSVC word concept diagnostics differently,
// so pinning message text would make this suite non-portable.
//
// Built with -DCASE=<n>; every case here is expected to fail compilation.

#include "../../single_header/s2s.hpp"

using namespace s2s_literals;

using u16 = unsigned short;

#if CASE == 1
// TODO(045): must NOT compile — 42 is neither a size nor a constraint. The
// diagnostic should isolate this entry, name its type, and name both things it
// could have been ("no operand of the disjunction is satisfied"), which is what
// the per-element placeholder constraint buys over a fold in a requires-clause.
using unrecognised_entry =
  s2s::struct_field_list<
    s2s::basic_field<"version", u16, 42>
  >;
#endif

#if CASE == 2
// TODO(045): must NOT compile — two sizes in one pack. Order-independence means
// the classifier scans rather than reads positionally, so nothing about the
// grammar stops a second size being written; the duplicate-option count is what
// rejects it.
using duplicate_size =
  s2s::struct_field_list<
    s2s::basic_field<"version", u16, 2_B, 2_B>
  >;
#endif

auto main() -> int {
  return 0;
}
