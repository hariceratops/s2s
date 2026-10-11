// The view entry points reject what cannot be viewed at compile time.
// Design: dev/design/reading-two-fields-costs-a-whole-record.md §7.3.
//
// Built with -DCASE=<n>; with no CASE it compiles, which is the control that
// proves the valid spelling is accepted and each case fails on its own line.
// Matched on the name of the atom that failed, so no comment here may spell
// that name.

#include <array>
#include <span>
#include "../../single_header/s2s.hpp"

using namespace s2s_literals;

using u8 = unsigned char;
using u32 = unsigned int;

using record =
  s2s::struct_field_list<
    s2s::basic_field<"a", u32, 4_B>,
    s2s::basic_field<"b", u32, 4_B>
  >;

using with_array =
  s2s::struct_field_list<
    s2s::fixed_array_field<"arr", u8, 6>
  >;

auto main() -> int {
#ifndef CASE
  std::array<char, 8> exact{};
  std::array<char, 6> exact_arrays{};
  auto view = s2s::struct_view_le<record>(std::span<const char, 8>{exact});
  auto arrays = s2s::struct_view_le<with_array>(std::span<const char, 6>{exact_arrays});
  return view.has_value() && arrays.has_value() ? 0 : 1;
#endif

#if CASE == 1
  // Must NOT compile — a static extent one byte short of the record.
  std::array<char, 7> short_buffer{};
  auto view = s2s::struct_view_le<record>(std::span<const char, 7>{short_buffer});
  return view.has_value() ? 0 : 1;
#endif

#if CASE == 2
  // Must NOT compile — an accessor returns a const value, so a write through
  // it has nowhere to go.
  std::array<char, 6> buffer{};
  auto view = s2s::struct_view_le<with_array>(std::span<const char, 6>{buffer});
  (*view)["arr"_f] = std::array<u8, 6>{};
  return 0;
#endif
}
