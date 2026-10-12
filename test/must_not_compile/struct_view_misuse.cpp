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

using inner_header =
  s2s::struct_field_list<
    s2s::basic_field<"kind", u8, 1_B>,
    s2s::basic_field<"len", u32, 4_B>
  >;

#if CASE == 3
// Must NOT compile — a vector whose length is read from the stream.
using rejected =
  s2s::struct_field_list<
    s2s::basic_field<"n", u8, 1_B>,
    s2s::vec_field<"v", u8, s2s::len_from_field<"n">>
  >;
#elif CASE == 4
// Must NOT compile — a string whose length is read from the stream.
using rejected =
  s2s::struct_field_list<
    s2s::basic_field<"n", u8, 1_B>,
    s2s::str_field<"s", s2s::len_from_field<"n">>
  >;
#elif CASE == 5
// Must NOT compile — a run of records whose count is read from the stream.
using rejected =
  s2s::struct_field_list<
    s2s::basic_field<"n", u8, 1_B>,
    s2s::vector_of_records<"r", inner_header, s2s::len_from_field<"n">>
  >;
#elif CASE == 6
// Must NOT compile — a field that may be absent.
using rejected =
  s2s::struct_field_list<
    s2s::basic_field<"present", u8, 1_B>,
    s2s::maybe<
      s2s::basic_field<"opt", u32, 4_B>,
      s2s::parse_if<[](u8 present) { return present != 0; }, "present">
    >
  >;
#elif CASE == 7 || CASE == 11
// Alternatives that all share one width.
using alt_x = s2s::struct_field_list<s2s::basic_field<"x", u32, 4_B>>;
using alt_y = s2s::struct_field_list<s2s::basic_field<"y", u32, 4_B>>;
using same_width_variant =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::variance<"body",
      s2s::type<s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<1, s2s::as_struct<alt_x>>,
          s2s::match_case<2, s2s::as_struct<alt_y>>>>>
  >;
#if CASE == 7
// Must NOT compile — a same-width variance is still a data-dependent choice.
using rejected = same_width_variant;
#else
// Must NOT compile — the variance sits an array element deep.
using rejected =
  s2s::struct_field_list<s2s::array_of_records<"items", same_width_variant, 2>>;
#endif
#elif CASE == 8
using marker_record =
  s2s::struct_field_list<s2s::fixed_array_field<"marker", u8, 2>>;
// Must NOT compile — the record decides its own byte order.
using rejected =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", marker_record,
      s2s::order_from<s2s::match_field<"marker">,
        s2s::order_switch<
          s2s::order_case<std::array<u8, 2>{'I', 'I'}, std::endian::little>,
          s2s::order_case<std::array<u8, 2>{'M', 'M'}, std::endian::big>>>>,
    s2s::basic_field<"magic", u32>
  >;
#elif CASE == 9
// Must NOT compile — the optional field sits one record deep.
using rejected =
  s2s::struct_field_list<
    s2s::basic_field<"lead", u8, 1_B>,
    s2s::struct_field<"inner",
      s2s::struct_field_list<
        s2s::basic_field<"present", u8, 1_B>,
        s2s::maybe<
          s2s::basic_field<"opt", u32, 4_B>,
          s2s::parse_if<[](u8 present) { return present != 0; }, "present">
        >
      >>
  >;
#elif CASE == 10
// Must NOT compile — the variable-width field sits inside an array element.
using rejected =
  s2s::struct_field_list<
    s2s::array_of_records<"items",
      s2s::struct_field_list<
        s2s::basic_field<"n", u8, 1_B>,
        s2s::vec_field<"v", u8, s2s::len_from_field<"n">>
      >, 2>
  >;
#endif

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

#if CASE >= 3
  std::array<char, 64> buffer{};
  auto view = s2s::struct_view_le<rejected>(std::span<const char, 64>{buffer});
  return view.has_value() ? 0 : 1;
#endif
}
