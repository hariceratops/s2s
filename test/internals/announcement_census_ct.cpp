// The announcement census — the load-bearing metaprogram behind 056, 058
// and 060.
// Design: dev/design/byte-order-is-data-not-a-template-parameter.md §10
//
// Lives here rather than under schema/ because it asserts a type-level
// computation, not a round-trip — the same reason union_choice_pipeline_ct
// does.
//
// Why it exists at all: every criterion in 056, 058 and 060 is negative
// ("fails to compile"), and a negative cannot distinguish a census that
// counted correctly from one that counted wrongly in a way nothing observes
// yet. This file is the positive form.
//
// The off-spine schemas are asserted as *field packs*, never wrapped in a
// struct_field_list: 057 rejects an off-spine announcement and 060 a second
// one, so a list built around either would stop compiling as those slices land.
// census_of_fields is what those rejections will read, and it is what is
// asserted here.
//
// Verify it is non-vacuous by flipping one expected count and watching it fail.
//
// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture.

#include <array>
#include <bit>
#include <string>
#include <vector>
#include <ut>

#include "../../include/s2s.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

using order_marker =
  s2s::struct_field_list<
    s2s::fixed_array_field<"marker", u8, 2>
  >;

using order_guide =
  s2s::order_from<s2s::match_field<"marker">,
    s2s::order_switch<
      s2s::order_case<std::array<u8, 2>{'I', 'I'}, std::endian::little>,
      s2s::order_case<std::array<u8, 2>{'M', 'M'}, std::endian::big>>>;

using announcement = s2s::announces_byte_order<"byte_order", order_marker, order_guide>;

using flat =
  s2s::struct_field_list<
    s2s::basic_field<"a", u32>,
    s2s::basic_field<"b", u32>
  >;

using announcing =
  s2s::struct_field_list<
    announcement,
    s2s::basic_field<"magic", u32>
  >;

using announcing_one_deep =
  s2s::struct_field_list<
    s2s::struct_field<"header", announcing>
  >;

// The counts, asserted directly. A schema declares at most one announcement, so
// every count above one below is a pack that no struct_field_list is built
// from.
static_assert(s2s::census_of_fields<>::value.on_spine == 0);
static_assert(s2s::census_of_list_v<flat>.on_spine == 0);
static_assert(s2s::census_of_list_v<flat>.off_spine == 0);

static_assert(s2s::census_of_list_v<announcing>.on_spine == 1);
static_assert(s2s::census_of_list_v<announcing>.off_spine == 0);

// Two records deep, which is what proves the walk is not one level of struct.
static_assert(s2s::census_of_list_v<announcing_one_deep>.on_spine == 1);
static_assert(
  s2s::census_of_fields<s2s::struct_field<"outer", announcing_one_deep>>::value.on_spine == 1);

// Reached conditionally or repeatedly: the count moves to off_spine, which is
// the distinction 057 rejects on. The announcement itself is unchanged in each.
static_assert(
  s2s::census_of_fields<
    s2s::maybe<s2s::struct_field<"header", announcing>, s2s::always_present>
  >::value.off_spine == 1);
static_assert(
  s2s::census_of_fields<
    s2s::maybe<s2s::struct_field<"header", announcing>, s2s::always_present>
  >::value.on_spine == 0);

static_assert(
  s2s::census_of_fields<s2s::array_of_records<"headers", announcing, 3>>::value.off_spine == 1);
static_assert(
  s2s::census_of_fields<s2s::array_of_records<"headers", announcing, 3>>::value.on_spine == 0);

static_assert(
  s2s::census_of_fields<
    s2s::vector_of_records<"headers", announcing, s2s::len_from_field<"n">>
  >::value.off_spine == 1);

using in_an_alternative =
  s2s::variance<"body",
    s2s::type<s2s::match_field<"tag">,
      s2s::type_switch<
        s2s::match_case<1, s2s::as_struct<announcing>>,
        s2s::match_case<2, s2s::as_trivial<u32, 4_B>>>>>;

static_assert(s2s::census_of_fields<in_an_alternative>::value.off_spine == 1);
static_assert(s2s::census_of_fields<in_an_alternative>::value.on_spine == 0);

// Cumulative rather than per-level: the two sit in different subtrees and
// neither level below sees both. This is what 060 rejects on.
static_assert(
  s2s::census_of_fields<
    s2s::struct_field<"left", announcing>,
    s2s::struct_field<"right", announcing>
  >::value.on_spine == 2);

// An announcement inside an announced record is counted, not hidden by the
// field that carries it.
static_assert(s2s::census_of_field<announcement>().on_spine == 1);


// The order-agnostic predicate, one assertion per row of design §4.1's table.
static_assert(s2s::is_order_agnostic_value<u8>());
static_assert(!s2s::is_order_agnostic_value<u16>());
static_assert(!s2s::is_order_agnostic_value<float>());
static_assert(s2s::is_order_agnostic_value<s2s::fixed_string<4>>());
static_assert(s2s::is_order_agnostic_value<std::string>());
static_assert(s2s::is_order_agnostic_value<std::array<u8, 2>>());
static_assert(!s2s::is_order_agnostic_value<std::array<u16, 2>>());
static_assert(s2s::is_order_agnostic_value<std::array<std::array<u8, 2>, 2>>());
static_assert(!s2s::is_order_agnostic_value<std::array<std::array<u16, 2>, 2>>());
static_assert(s2s::is_order_agnostic_value<char[8]>());
static_assert(!s2s::is_order_agnostic_value<u32[8]>());
static_assert(s2s::is_order_agnostic_value<std::vector<u8>>());
static_assert(!s2s::is_order_agnostic_value<std::vector<u32>>());
static_assert(s2s::is_order_agnostic_value<order_marker>());
static_assert(!s2s::is_order_agnostic_value<flat>());

// On the field, so the wrapper kinds are covered too.
static_assert(s2s::is_order_agnostic_field<s2s::fixed_array_field<"m", u8, 2>>());
static_assert(!s2s::is_order_agnostic_field<s2s::basic_field<"v", u16, 2_B>>());
static_assert(s2s::is_order_agnostic_field<s2s::magic_byte_array<"m", 2, std::array<u8, 2>{'I', 'I'}>>());
static_assert(s2s::is_order_agnostic_field<s2s::fixed_string_field<"tag", 3>>());
static_assert(!s2s::is_order_agnostic_field<s2s::struct_field<"s", flat>>());
static_assert(s2s::is_order_agnostic_field<s2s::struct_field<"s", order_marker>>());
static_assert(!s2s::is_order_agnostic_field<s2s::array_of_records<"a", flat, 2>>());
static_assert(s2s::is_order_agnostic_field<s2s::array_of_records<"a", order_marker, 2>>());
static_assert(
  !s2s::is_order_agnostic_field<
    s2s::maybe<s2s::basic_field<"v", u16, 2_B>, s2s::always_present>>());
static_assert(
  s2s::is_order_agnostic_field<
    s2s::maybe<s2s::basic_field<"v", u8, 1_B>, s2s::always_present>>());

// A union is agnostic only if every alternative is — the reader cannot know
// which one it will take until the discriminant is read.
static_assert(
  !s2s::is_order_agnostic_field<
    s2s::variance<"body",
      s2s::type<s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<1, s2s::as_trivial<u8, 1_B>>,
          s2s::match_case<2, s2s::as_trivial<u32, 4_B>>>>>>());
static_assert(
  s2s::is_order_agnostic_field<
    s2s::variance<"body",
      s2s::type<s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<1, s2s::as_trivial<u8, 1_B>>,
          s2s::match_case<2, s2s::as_fixed_arr<u8, 4>>>>>>());

auto main() -> int {
  // Everything this file checks is a static_assert over a type-level
  // computation, so there is nothing to run. ut still needs a case: the
  // *_coverage entry compares compile-time and run-time counts, and a binary
  // with none of either fails add_ut_test's guard against a suite that ran
  // nothing.
  "the census and the order-agnostic predicate are asserted above"_test = [] constexpr {
    expect(eq(s2s::census_of_list_v<announcing>.on_spine, std::size_t{1}));
  };
}
