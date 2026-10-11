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

auto main() -> int {
  "the layout table is a compile-time prefix sum"_test = [] constexpr {
    expect(eq(s2s::wire_width<flat>, std::size_t{14}));
    expect(eq(s2s::record_layout<flat>::offsets[4], std::size_t{10}));
  };
}
