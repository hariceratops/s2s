// until<> (062-064) rejects spellings a delimited read cannot give a sound
// answer to.
//
// Assert only that the build fails: the diagnostic is a named concept, not
// pinned text, per this directory's own policy — concept wording differs
// across gcc, clang and MSVC, and every rejection here is expressible as a
// concept rather than as a count, which is what the design reserves
// static_assert for.
//
// Built with -DCASE=<n>; every case here is expected to fail compilation.

#include "../../single_header/s2s.hpp"

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

#if CASE == 1
// Must NOT compile — a delimiter is one byte, and matching it against a u32
// element raises partial-match and alignment questions no format asks.
// Rejected by delimited_buffer_is_byte_wide.
using wide_element_delimited_vector =
  s2s::struct_field_list<
    s2s::vec_field<"data", u32, s2s::until<u8{0}>>
  >;
#elif CASE == 2
// Must NOT compile — a delimiter is one byte to match against a stream byte,
// and a two-byte value has no single-byte comparison to make. Rejected by
// delimiter_value_like.
using multi_byte_delimiter =
  s2s::struct_field_list<
    s2s::str_field<"keyword", s2s::until<u16{0x0d0a}>>
  >;
#elif CASE == 3
// Must NOT compile — a basic_field's size axis is fixed only; a delimiter
// reads until a byte is seen, which basic_field's fits-check cannot evaluate.
// Rejected by fixed_size_like.
using until_on_basic_field =
  s2s::struct_field_list<
    s2s::basic_field<"count", u32, s2s::until<u8{0}>>
  >;
#elif CASE == 4
// Must NOT compile — fixed_array_field takes a constraint-only option pack,
// so until<> is an unrecognised pack entry, not a size.
using until_on_fixed_array_field =
  s2s::struct_field_list<
    s2s::fixed_array_field<"data", u8, 4, s2s::until<u8{0}>>
  >;
#elif CASE == 5
// Must NOT compile — c_str_field takes a constraint-only option pack, so
// until<> is an unrecognised pack entry, not a size.
using until_on_c_str_field =
  s2s::struct_field_list<
    s2s::c_str_field<"keyword", 8, s2s::until<u8{0}>>
  >;
#endif

auto main() -> int {
  return 0;
}
