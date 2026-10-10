// A floating-point field has one representation, so unlike an integer it
// cannot be declared narrower or wider than its type, and a type with no
// swappable width is refused at the schema rather than at the leaf. Both go
// through field_fits_to_underlying_type, which as_trivial shares.
//
// Assert only that the build fails; pinning the compiler's concept wording
// would make this suite non-portable.
//
// Built with -DCASE=<n>; with no CASE it compiles, which is the control that
// proves the valid spellings are accepted and each case fails on its own line.

#include "../../single_header/s2s.hpp"

using namespace s2s_literals;

using u8 = unsigned char;

#ifndef CASE
// The control: every valid spelling, so each case below fails on its own line.
using valid_floats =
  s2s::struct_field_list<
    s2s::basic_field<"a", float, 4_B>,
    s2s::basic_field<"b", double, 8_B>,
    s2s::basic_field<"c", float>,
    s2s::basic_field<"d", double>
  >;
#endif

#if CASE == 1
// Must NOT compile — narrower than the float. An integer may do this; a float
// would be read into half a representation.
using narrow_float =
  s2s::struct_field_list<
    s2s::basic_field<"a", float, 2_B>
  >;
#endif

#if CASE == 2
// Must NOT compile — wider than the float.
using wide_float =
  s2s::struct_field_list<
    s2s::basic_field<"a", float, 8_B>
  >;
#endif

#if CASE == 3
using narrow_double =
  s2s::struct_field_list<
    s2s::basic_field<"a", double, 4_B>
  >;
#endif

#if CASE == 4
// Must NOT compile — 16_B is never the width of a long double that
// byteswapped can swap: where it is 16 bytes wide it has no 2/4/8 width, and
// where it is a double it is not 16.
using long_double_field =
  s2s::struct_field_list<
    s2s::basic_field<"a", long double, 16_B>
  >;
#endif

#if CASE == 5
// Must NOT compile — the union tag shares the size check, so a narrow float is
// refused there as well.
using narrow_float_in_tag =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u8, 1_B>,
    s2s::variance<"body", s2s::type<s2s::match_field<"tag">,
      s2s::type_switch<
        s2s::match_case<0x01, s2s::as_trivial<float, 2_B>>
      >>>
  >;
#endif

auto main() -> int {
  return 0;
}
