// Positive form of the fixed-layout rule and the layout table that views read
// offsets from. Lives here rather than under schema/ because it asserts a
// type-level computation, not a read.
// Design: dev/design/reading-two-fields-costs-a-whole-record.md §2, §3.
//
// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture.

#include <array>
#include <ut>

#include "../../include/s2s.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

using flat =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u8, 1_B>,
    s2s::basic_field<"len", u32, 4_B>,
    s2s::basic_field<"flags", u16, 2_B>,
    s2s::basic_field<"narrow", u32, 3_B>,
    s2s::basic_field<"ratio", float, 4_B>
  >;

using aggregates =
  s2s::struct_field_list<
    s2s::fixed_array_field<"arr", u16, 3>,
    s2s::c_arr_field<"carr", u32, 2>,
    s2s::c_str_field<"cs", 3>,
    s2s::fixed_string_field<"fs", 4>
  >;

static_assert(s2s::fixed_layout_schema<flat>);
static_assert(s2s::fixed_layout_schema<aggregates>);
static_assert(s2s::wire_width<aggregates> == 6 + 8 + 4 + 5);
static_assert(s2s::array_of_trivials<std::array<u16, 3>>);
static_assert(!s2s::array_of_trivials<u16[3]>);
static_assert(s2s::c_array_of_trivials<u16[3]>);
static_assert(!s2s::c_array_of_trivials<std::array<u16, 3>>);
static_assert(s2s::viewable_leaf_value<s2s::fixed_string<4>>);
static_assert(!s2s::viewable_leaf_value<std::vector<u8>>);
static_assert(s2s::every_field_has_compile_time_width<flat>);

using constrained =
  s2s::struct_field_list<
    s2s::magic_number<"num", u32, 4_B, 0xdeadbeef>,
    s2s::magic_string<"str", "GIF">,
    s2s::magic_byte_array<"arr", 2, std::array<unsigned char, 2>{0xca, 0xfe}>,
    s2s::basic_field<"v", u16, 2_B, s2s::gte(7u)>
  >;
static_assert(s2s::fixed_layout_schema<constrained>);
static_assert(s2s::wire_width<constrained> == 4 + 4 + 2 + 2);
static_assert(s2s::any_field_constrained_v<constrained>);
static_assert(!s2s::any_field_constrained_v<flat>);
static_assert(!s2s::any_field_constrained_v<int>);

static_assert(s2s::wire_width<flat> == 14);
static_assert(s2s::record_layout<flat>::offsets == std::array<std::size_t, 5>{0, 1, 5, 7, 10});
static_assert(s2s::record_layout<flat>::widths == std::array<std::size_t, 5>{1, 4, 2, 3, 4});
static_assert(s2s::record_layout<flat>::names[3] == "narrow");

static_assert(s2s::is_no_constraint_v<s2s::no_constraint<u32>>);
static_assert(!s2s::is_no_constraint_v<s2s::eq<u32>>);
static_assert(!s2s::is_no_constraint_v<u32>);

static_assert(s2s::raw_byte_like<char>);
static_assert(s2s::raw_byte_like<const unsigned char>);
static_assert(s2s::raw_byte_like<const std::byte>);
static_assert(!s2s::raw_byte_like<signed char>);
static_assert(!s2s::raw_byte_like<int>);

static_assert(s2s::span_covers_record<std::dynamic_extent, flat>);
static_assert(s2s::span_covers_record<14, flat>);
static_assert(s2s::span_covers_record<64, flat>);
static_assert(!s2s::span_covers_record<13, flat>);

// The constraint must be false, not a hard error, for a type that is no schema.
static_assert(!s2s::fixed_layout_schema<int>);

using view_t = decltype(*s2s::struct_view_le<flat>(std::span<const u8>{}));
static_assert(std::is_trivially_copyable_v<std::remove_cvref_t<view_t>>);
static_assert(sizeof(std::remove_cvref_t<view_t>) == sizeof(const u8*));

// Nesting (078): widths and offsets recurse, and a constraint anywhere inside
// counts as a constraint on the record.
using inner_header =
  s2s::struct_field_list<
    s2s::basic_field<"kind", u8, 1_B>,
    s2s::basic_field<"len", u32, 4_B>
  >;
using nested_record =
  s2s::struct_field_list<
    s2s::basic_field<"lead", u8, 1_B>,
    s2s::struct_field<"hdr", inner_header>,
    s2s::basic_field<"tail", u16, 2_B>
  >;
using inner_checked =
  s2s::struct_field_list<
    s2s::basic_field<"len", u32, 4_B, s2s::gte(7u)>
  >;
using nested_checked =
  s2s::struct_field_list<
    s2s::basic_field<"lead", u8, 1_B>,
    s2s::struct_field<"hdr", inner_checked>
  >;
static_assert(s2s::fixed_layout_schema<nested_record>);
static_assert(s2s::wire_width<nested_record> == 1 + 5 + 2);
static_assert(s2s::record_layout<nested_record>::offsets == std::array<std::size_t, 3>{0, 1, 6});
static_assert(!s2s::any_field_constrained_v<nested_record>);
static_assert(s2s::any_field_constrained_v<nested_checked>);
using inner_variable =
  s2s::struct_field_list<
    s2s::basic_field<"n", u8, 1_B>,
    s2s::str_field<"s", s2s::len_from_field<"n">>
  >;
static_assert(!s2s::fixed_layout_schema<
  s2s::struct_field_list<s2s::struct_field<"v", inner_variable>>>);

// The rejected kinds (080): the concept and the sub-concept that names the rule
// are false, not a hard error, and the others stay true.
using with_vec =
  s2s::struct_field_list<
    s2s::basic_field<"n", u8, 1_B>,
    s2s::vec_field<"v", u8, s2s::len_from_field<"n">>
  >;
using with_str =
  s2s::struct_field_list<
    s2s::basic_field<"n", u8, 1_B>,
    s2s::str_field<"s", s2s::len_from_field<"n">>
  >;
using with_records =
  s2s::struct_field_list<
    s2s::basic_field<"n", u8, 1_B>,
    s2s::vector_of_records<"r", inner_header, s2s::len_from_field<"n">>
  >;
using with_maybe =
  s2s::struct_field_list<
    s2s::basic_field<"present", u8, 1_B>,
    s2s::maybe<
      s2s::basic_field<"opt", u32, 4_B>,
      s2s::parse_if<[](u8 present) { return present != 0; }, "present">
    >
  >;
using same_width_a = s2s::struct_field_list<s2s::basic_field<"x", u32, 4_B>>;
using same_width_b = s2s::struct_field_list<s2s::basic_field<"y", u32, 4_B>>;
using with_variance =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::variance<"body",
      s2s::type<s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<1, s2s::as_struct<same_width_a>>,
          s2s::match_case<2, s2s::as_struct<same_width_b>>>>>
  >;
using marker_record =
  s2s::struct_field_list<s2s::fixed_array_field<"marker", u8, 2>>;
using with_announcer =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", marker_record,
      s2s::order_from<s2s::match_field<"marker">,
        s2s::order_switch<
          s2s::order_case<std::array<u8, 2>{'I', 'I'}, std::endian::little>,
          s2s::order_case<std::array<u8, 2>{'M', 'M'}, std::endian::big>>>>,
    s2s::basic_field<"magic", u32>
  >;

static_assert(s2s::has_no_announcing_record<flat>);
static_assert(s2s::has_no_optional_field<flat>);
static_assert(s2s::has_no_variant_field<flat>);
static_assert(s2s::every_field_has_compile_time_width<flat>);
static_assert(s2s::has_no_announcing_record<nested_record>);
static_assert(s2s::has_no_optional_field<nested_record>);
static_assert(s2s::has_no_variant_field<nested_record>);

static_assert(!s2s::every_field_has_compile_time_width<with_vec>);
static_assert(!s2s::every_field_has_compile_time_width<with_str>);
static_assert(!s2s::every_field_has_compile_time_width<with_records>);
static_assert(!s2s::fixed_layout_schema<with_vec>);
static_assert(!s2s::fixed_layout_schema<with_str>);
static_assert(!s2s::fixed_layout_schema<with_records>);
static_assert(s2s::has_no_announcing_record<with_vec>);
static_assert(s2s::has_no_optional_field<with_vec>);
static_assert(s2s::has_no_variant_field<with_vec>);

static_assert(!s2s::has_no_optional_field<with_maybe>);
static_assert(!s2s::fixed_layout_schema<with_maybe>);
static_assert(s2s::has_no_variant_field<with_maybe>);

static_assert(!s2s::has_no_variant_field<with_variance>);
static_assert(!s2s::fixed_layout_schema<with_variance>);
static_assert(s2s::has_no_optional_field<with_variance>);

static_assert(!s2s::has_no_announcing_record<with_announcer>);
static_assert(!s2s::fixed_layout_schema<with_announcer>);

// The check descends through struct_field and array_of_records.
using maybe_in_record =
  s2s::struct_field_list<s2s::struct_field<"m", with_maybe>>;
using variance_in_record =
  s2s::struct_field_list<s2s::struct_field<"v", with_variance>>;
using announcer_in_record =
  s2s::struct_field_list<s2s::struct_field<"a", with_announcer>>;
using vec_in_record =
  s2s::struct_field_list<s2s::struct_field<"v", with_vec>>;
using maybe_in_array =
  s2s::struct_field_list<s2s::array_of_records<"m", with_maybe, 2>>;
using variance_in_array =
  s2s::struct_field_list<s2s::array_of_records<"v", with_variance, 2>>;
using vec_in_array =
  s2s::struct_field_list<s2s::array_of_records<"v", with_vec, 2>>;
static_assert(!s2s::has_no_optional_field<maybe_in_record>);
static_assert(!s2s::has_no_variant_field<variance_in_record>);
static_assert(!s2s::has_no_announcing_record<announcer_in_record>);
static_assert(!s2s::every_field_has_compile_time_width<vec_in_record>);
static_assert(!s2s::has_no_optional_field<maybe_in_array>);
static_assert(!s2s::has_no_variant_field<variance_in_array>);
static_assert(!s2s::every_field_has_compile_time_width<vec_in_array>);
static_assert(!s2s::fixed_layout_schema<maybe_in_record>);
static_assert(!s2s::fixed_layout_schema<vec_in_array>);

// Each sub-concept is pure for a type that is no schema.
static_assert(!s2s::has_no_announcing_record<int>);
static_assert(!s2s::has_no_optional_field<int>);
static_assert(!s2s::has_no_variant_field<int>);
static_assert(!s2s::every_field_has_compile_time_width<int>);

auto main() -> int {
  "the layout table is a compile-time prefix sum"_test = [] constexpr {
    expect(eq(s2s::wire_width<flat>, std::size_t{14}));
    expect(eq(s2s::record_layout<flat>::offsets[4], std::size_t{10}));
  };
}
