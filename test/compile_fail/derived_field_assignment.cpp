// Compile-fail test: assigning to a derived field must not compile, while
// reading it through the const accessor must.
//
// Built with -DCASE=<n>; each case is expected to fail compilation except
// CASE_READS_OK, which must succeed. See CMakeLists.txt.

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
  // TODO(issue 005): must NOT compile — "len" is a len_from_field target,
  // so the non-const operator[] is constrained away and the const overload
  // is selected, yielding an assign-to-const error.
  obj["len"_f] = 5;
#elif CASE == 2
  // TODO(issue 009): must NOT compile — a type_switch discriminant is
  // derived from the held alternative. Replace our_struct with a variance
  // schema and assign to its discriminant field.
  obj["len"_f] = 5;
#elif CASE == 3
  // Must COMPILE — const read of a derived field stays available, and a
  // non-derived field stays assignable.
  const auto& len = std::as_const(obj)["len"_f];
  (void)len;
  obj["str"_f] = "hello";
#endif

  return 0;
}
