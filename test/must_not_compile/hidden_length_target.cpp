// Reading a len_from_field target must not compile once 043 lands.
//
// Distinct from derived_field_assignment.cpp CASE 1, which pins the *current*
// behaviour: a length target is readable and rejects assignment as
// assign-to-const. After 043 the overload is gone entirely, so naming the field
// at all is a no-such-member error. Both files must exist and both must fail —
// derived_field_assignment.cpp CASE 2 keeps covering assign-to-const for a
// *discriminant*, which 043 deliberately leaves alone.
//
// Built with -DCASE=<n>; every case here is expected to fail compilation.

#include "../../single_header/s2s.hpp"

using namespace s2s_literals;

using u32 = unsigned int;

using our_struct =
  s2s::struct_field_list<
    s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::str_field<"str", s2s::field_size<s2s::len_from_field<"len">>>
  >;

auto main() -> int {
  our_struct obj{};

#if CASE == 1
  // TODO(043): must NOT compile — "len" is a length-derived target and has no
  // operator[] overload at all after 043. Assert the diagnostic is
  // no-such-member, not assign-to-const; the const-read overload this replaces
  // is what makes the two distinguishable.
  auto len = obj["len"_f];
  (void)len;
#endif

  // TODO(043): the positive half — assigning "str" and letting the write path
  // derive "len" — belongs in test/schema/size_axis_write_ct.cpp, since it is
  // code that compiles.

  return 0;
}
