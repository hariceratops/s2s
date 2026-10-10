// until_field_equals<> (069-070) rejects declarations whose terminator cannot
// be synthesised on write, or which state two things about what ends the run.
//
// Assert only that the build fails: the diagnostic is a named concept or a
// static_assert, not pinned text, per this directory's own policy.
//
// Built with -DCASE=<n>; every case here is expected to fail compilation.

#include "../../single_header/s2s.hpp"

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;

using sub_block =
  s2s::struct_field_list<
    s2s::basic_field<"size", u8, 1_B>,
    s2s::vec_field<"data", u8, s2s::len_from_field<"size">>
  >;

#if CASE == 1
// Must NOT compile - a sibling-field count and a sentinel are two size
// options; a field takes at most one.
using count_and_sentinel =
  s2s::struct_field_list<
    s2s::basic_field<"n", u8, 1_B>,
    s2s::vector_of_records<"blocks", sub_block,
      s2s::len_from_field<"n">, s2s::until_field_equals<"size", u8{0}>>
  >;
#elif CASE == 2
// Must NOT compile - a literal count and a sentinel, the same contradiction.
using literal_count_and_sentinel =
  s2s::struct_field_list<
    s2s::vector_of_records<"blocks", sub_block,
      3_B, s2s::until_field_equals<"size", u8{0}>>
  >;
#elif CASE == 3
// Must NOT compile - a computed size is verified against its container rather
// than derived, and the callable is not evaluable here, so the terminating
// element's width is not determined. Rejected by terminator_field_check.
constexpr auto always_two = [](u8) -> std::size_t { return 2; };

using computed_size_element =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u8, 1_B>,
    s2s::basic_field<"size", u8, 1_B>,
    s2s::vec_field<"data", u8, s2s::size_from_fields<always_two, "tag">>
  >;

using computed_size_run =
  s2s::struct_field_list<
    s2s::vector_of_records<"blocks", computed_size_element,
      s2s::until_field_equals<"size", u8{0}>>
  >;
#elif CASE == 4
// Must NOT compile - the sentinel field is the length of "data", so the write
// path derives it from the empty container and writes zero, never 0xff.
// Rejected by terminator_synthesis_check's derivation obligation.
using nonzero_sentinel_on_length_target =
  s2s::struct_field_list<
    s2s::vector_of_records<"blocks", sub_block,
      s2s::until_field_equals<"size", u8{0xff}>>
  >;
#endif

auto main() -> int {
  return 0;
}
