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
//
// The first alternative also carries a constraint, so the two things 050 has to
// keep apart are visible in one comparison: a declared constraint reaches the
// field, and a sibling that declared none keeps no_constraint rather than
// inheriting its neighbour's.
using body_field = s2s::variance<
  "body",
  s2s::type<
    s2s::match_field<"tag">,
    s2s::type_switch<
      s2s::match_case<0x01, s2s::as_trivial<u32, 4_B, s2s::lte{99u}>>,
      s2s::match_case<0x02, s2s::as_struct<inner>>,
      s2s::match_case<0x03, s2s::as_vec<u8, s2s::len_from_field<"n">>>
    >
  >
>;

// The same three alternatives with the one two-entry pack written the other way
// round. Order-independence is a property of the whole option set, so the claim
// is type identity rather than equivalent behaviour.
using body_field_reversed = s2s::variance<
  "body",
  s2s::type<
    s2s::match_field<"tag">,
    s2s::type_switch<
      s2s::match_case<0x01, s2s::as_trivial<u32, s2s::lte{99u}, 4_B>>,
      s2s::match_case<0x02, s2s::as_struct<inner>>,
      s2s::match_case<0x03, s2s::as_vec<u8, s2s::len_from_field<"n">>>
    >
  >
>;

// The four tags whose size is fixed by the tag itself gained their pack in 050,
// next to the enforcement that makes it mean something. They have no
// order-independence to demonstrate — one admissible entry — so the claim is
// only that the entry resolves and reaches `field`.
struct nonzero_first {
  constexpr auto operator()(const std::array<u8, 2>& arr) const -> bool {
    return arr[0] != 0;
  }
};

struct nonzero_x {
  constexpr auto operator()(const inner& record) const -> bool {
    return record["x"_f] != 0;
  }
};

struct nonempty {
  constexpr auto operator()(const s2s::fixed_string<3>& s) const -> bool {
    return s.value[0] != '\0';
  }
};

struct first_record_nonzero {
  constexpr auto operator()(const std::array<inner, 2>& records) const -> bool {
    return records[0]["x"_f] != 0;
  }
};

using sizeless_tags = s2s::variance<
  "body",
  s2s::type<
    s2s::match_field<"tag">,
    s2s::type_switch<
      s2s::match_case<0x01, s2s::as_fixed_arr<u8, 2, nonzero_first{}>>,
      s2s::match_case<0x02, s2s::as_struct<inner, nonzero_x{}>>,
      s2s::match_case<0x03, s2s::as_fixed_string<3, nonempty{}>>,
      s2s::match_case<0x04, s2s::as_arr_of_records<inner, 2, first_record_nonzero{}>>
    >
  >
>;

using expected_sizeless_choices =
  s2s::field_choice_list<
    s2s::field<"body", std::array<u8, 2>, s2s::byte_count{2}, nonzero_first{}>,
    s2s::field<"body", inner, s2s::size_dont_care, nonzero_x{}>,
    s2s::field<"body", s2s::fixed_string<3>, s2s::byte_count{4}, nonempty{}>,
    s2s::field<"body", std::array<inner, 2>, s2s::size_dont_care, first_record_nonzero{}>
  >;

using defaulted_size = s2s::variance<
  "body",
  s2s::type<
    s2s::match_field<"tag">,
    s2s::type_switch<
      s2s::match_case<0x01, s2s::as_trivial<u32>>
    >
  >
>;

// Spelled out rather than derived, so that a change to the pipeline shows up
// as a mismatch here instead of propagating into both sides of the comparison.
using expected_choices =
  s2s::field_choice_list<
    s2s::field<"body", u32, 4_B, s2s::lte{99u}>,
    s2s::field<"body", inner, s2s::size_dont_care, s2s::no_constraint<inner>{}>,
    s2s::field<"body", std::vector<u8>, s2s::len_from_field<"n">,
               s2s::no_constraint<std::vector<u8>>{}>
  >;

// Reads the built list rather than the spelled-out one: a constraint that is
// callable only in expected_choices would prove nothing about the pipeline.
template <typename choices>
struct first_choice;

template <typename head, typename... tail>
struct first_choice<s2s::field_choice_list<head, tail...>> {
  using type = head;
};

template <typename choices>
using first_choice_of = typename first_choice<choices>::type;

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

  // New capability arriving as a side effect of the size becoming a pack
  // entry: a tag with an empty pack takes the same default its mirror
  // descriptor does. as_trivial<u32> was an arity error before 049.
  "a trivial tag with no size entry defaults to sizeof"_test = [] constexpr {
    expect(eq(std::is_same_v<
      typename defaulted_size::field_choices,
      s2s::field_choice_list<
        s2s::field<"body", u32, s2s::byte_count{sizeof(u32)}, s2s::no_constraint<u32>{}>
      >>, true));
  };

  // The constraint has to survive as a callable, not merely as a matching
  // template argument: it is what read_variant_impl and write_variant_impl
  // invoke, and a type-level comparison alone would not catch a constraint that
  // arrived as the right value but could not be called.
  "a declared constraint arrives callable"_test = [] constexpr {
    using first = first_choice_of<typename body_field::field_choices>;
    expect(eq(first::constraint_checker(50u), true));
    expect(eq(first::constraint_checker(200u), false));
  };

  "the tags with no size entry carry a constraint too"_test = [] constexpr {
    expect(eq(std::is_same_v<typename sizeless_tags::field_choices,
                             expected_sizeless_choices>, true));
  };

  "the entries of one pack are order-independent"_test = [] constexpr {
    expect(eq(std::is_same_v<typename body_field_reversed::field_choices,
                             expected_choices>, true));
  };

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
