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

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

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

#if CASE == 3
// Must NOT compile — a bound is meaningless on a field whose extent
// is a template parameter, since nothing a stream says can change how much it
// allocates. It must fail the per-element placeholder constraint the same way
// an unrecognised entry does, not be silently ignored: silence would let a
// schema author believe they had bounded something they had not.
using bound_on_a_fixed_size_field =
  s2s::struct_field_list<
    s2s::basic_field<"version", u16, 2_B, s2s::max_bytes<4096>>
  >;
#endif

#if CASE == 4
// Must NOT compile — two bounds in one pack, caught by the same
// duplicate-count assertion pack_options already applies to sizes and
// constraints. Without it the scan would silently take the first.
using duplicate_bound =
  s2s::struct_field_list<
    s2s::basic_field<"n", u32, 4_B>,
    s2s::vec_field<"payload", u8, s2s::len_from_field<"n">,
                   s2s::max_bytes<4096>, s2s::max_bytes<8192>>
  >;
#endif

auto main() -> int {
  return 0;
}
