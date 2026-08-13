// Reading a len_from_field target must not compile.
//
// Not the same claim as derived_field_assignment.cpp, which now covers only the
// discriminant: that one is readable and rejects assignment as assign-to-const.
// A length target has no overload at all, so naming it is a no-such-member
// error — and the read below, not an assignment, is what pins the difference.
//
// Built with -DCASE=<n>; every case here is expected to fail compilation.

#include "../../single_header/s2s.hpp"

using namespace s2s_literals;

using u32 = unsigned int;

using our_struct =
  s2s::struct_field_list<
    s2s::basic_field<"len", u32, 4_B>,
    s2s::str_field<"str", s2s::len_from_field<"len">>
  >;

auto main() -> int {
  our_struct obj{};

#if CASE == 1
  // Must NOT compile — "len" is a length-derived target, so it has no
  // operator[] overload at all. A read rather than an assignment, because an
  // assignment would also fail against the old const-returning overload and
  // would not tell the two apart.
  auto len = obj["len"_f];
  (void)len;
#endif

  // The positive half — assigning "str" and letting the write path derive
  // "len" — lives in test/schema/size_axis_write_ct.cpp, since it compiles.

  return 0;
}
