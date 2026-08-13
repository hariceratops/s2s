// Assigning to a derived discriminant must not compile.
//
// The length-target half moved to hidden_length_target.cpp when 043 stopped
// exposing those fields at all: the claim is no longer "assigning fails" but
// "the field cannot be named", and a case asserting the old wording would go
// on passing for the wrong reason.
//
// Built with -DCASE=<n>; every case here is expected to fail compilation. The
// positive half — a derived field stays readable, its siblings assignable —
// lives in test/schema/size_axis_read_ct.cpp, since it is code that compiles.

#include "../../single_header/s2s.hpp"

using namespace s2s_literals;

using u32 = unsigned int;

using our_struct =
  s2s::struct_field_list<
    s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::str_field<"str", s2s::field_size<s2s::len_from_field<"len">>>
  >;

using alt_1 = s2s::struct_field_list<s2s::basic_field<"x", u32, s2s::field_size<s2s::fixed<4>>>>;
using alt_2 = s2s::struct_field_list<s2s::basic_field<"y", u32, s2s::field_size<s2s::fixed<4>>>>;

using union_struct =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0xcafed00d, s2s::as_struct<alt_1>>,
          s2s::match_case<0xdeadbeef, s2s::as_struct<alt_2>>
        >
      >
    >
  >;

#if CASE == 4
// Must NOT compile — two cases sharing a match value break round-trip
// silently: writing alt_2 emits 0xcafed00d, and reading it back selects alt_1.
using duplicate_value_struct =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0xcafed00d, s2s::as_struct<alt_1>>,
          s2s::match_case<0xcafed00d, s2s::as_struct<alt_2>>
        >
      >
    >
  >;
#endif

auto main() -> int {
  our_struct obj{};

#if CASE == 2
  // Must NOT compile — a type_switch discriminant is derived from the held
  // alternative, so the schema owns its value, not the caller.
  union_struct u{};
  u["tag"_f] = 5;
#elif CASE == 4
  // The schema above is the failure; nothing to do here.
  duplicate_value_struct dup{};
  (void)dup;
#endif

  return 0;
}
