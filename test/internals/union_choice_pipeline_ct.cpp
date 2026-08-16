// What the tag-to-field pipeline builds for a union's alternatives.
//
// 048 re-points `to_field_choices` from zipping a variant against a
// `size_choices_t` onto the type-condition list itself. Its stated criterion is
// that the existing suite passes unchanged — a negative check, which cannot
// tell "builds the same fields" apart from "builds different fields that happen
// to break no existing assertion".
//
// This file is the positive form. Every assertion below is true of the pipeline
// as it stands *before* 048 and must remain true after it, so the file is
// written once and read twice: as a description now, as a regression gate then.
// 050 and 051 extend it, since a per-alternative constraint and bound are
// visible here — in the fourth and fifth template arguments — before any stream
// is involved.
//
// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.

#include <type_traits>
#include <variant>
#include <vector>
#include <ut>

#include "../../include/s2s.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

using u8 = unsigned char;
using u32 = unsigned int;

using inner =
  s2s::struct_field_list<
    s2s::basic_field<"x", u32, 4_B>,
    s2s::basic_field<"y", u32, 4_B>
  >;

// One alternative of each shape the pipeline has to carry: a trivial with a
// byte size, a record with no size of its own, and a container whose size is
// derived from a sibling field. Named on its own rather than inside a
// struct_field_list because nothing here reads a stream — the "tag" and "n"
// siblings the deducer and the length refer to would never be consulted.
using body_field = s2s::variance<
  "body",
  s2s::type<
    s2s::match_field<"tag">,
    s2s::type_switch<
      s2s::match_case<0x01, s2s::as_trivial<u32, 4_B>>,
      s2s::match_case<0x02, s2s::as_struct<inner>>,
      s2s::match_case<0x03, s2s::as_vec<u8, s2s::len_from_field<"n">>>
    >
  >
>;

// Spelled out rather than derived, so that a change to the pipeline shows up
// as a mismatch here instead of propagating into both sides of the comparison.
using expected_choices =
  s2s::field_choice_list<
    s2s::field<"body", u32, 4_B, s2s::no_constraint<u32>{}>,
    s2s::field<"body", inner, s2s::size_dont_care, s2s::no_constraint<inner>{}>,
    s2s::field<"body", std::vector<u8>, s2s::len_from_field<"n">,
               s2s::no_constraint<std::vector<u8>>{}>
  >;

auto main() -> int {
  // The whole claim of 048 in one assertion: the alternatives the pipeline
  // builds, in order, with each one's id, type, size, constraint and bound.
  "the pipeline builds one field per alternative"_test = [] constexpr {
    expect(eq(std::is_same_v<typename body_field::field_choices, expected_choices>, true));
  };

  // Every alternative takes the union's id, not one of its own. This is why a
  // per-alternative constraint failure reports the union in `failed_at` — 050
  // asserts that from the read path; it originates here.
  "every alternative carries the union's id"_test = [] constexpr {
    expect(eq(std::variant_size_v<typename body_field::field_type>, std::size_t{3}));
    expect(eq(body_field::variant_size, std::size_t{3}));
  };

  // TODO(050): a constrained alternative resolves its constraint into the
  // fourth argument, while its siblings keep no_constraint. Extend
  // expected_choices rather than adding a second comparison.

  // TODO(051): a bounded alternative resolves its bound into the fifth
  // argument, and an unbounded sibling keeps use_default_bound. This is the
  // assertion that shows the bound reaches `field` without any read-path
  // plumbing, per the design's §7.

  // A union whose alternatives are reachable at all: as_arr_of_records and
  // as_vec_of_records have no is_type_tag specialization today, so naming one
  // in a match_case does not compile. 048 adds the specializations.
  // TODO(048): add an as_vec_of_records alternative here once it is nameable,
  // and assert its built field alongside the three above.
}
