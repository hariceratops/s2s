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
using u32 = unsigned int;

#if CASE == 1
// Must NOT compile — a delimiter is one byte, and matching it against a u32
// element raises partial-match and alignment questions no format asks.
// Rejected by delimited_buffer_is_byte_wide.
using wide_element_delimited_vector =
  s2s::struct_field_list<
    s2s::vec_field<"data", u32, s2s::until<u8{0}>>
  >;
#endif

auto main() -> int {
  return 0;
}
