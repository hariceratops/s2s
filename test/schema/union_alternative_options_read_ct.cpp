// Options declared on a union alternative, read path, compile-time tier.
//
// Its own pair rather than additions to union_read*, because the tree is
// organised by what a file verifies: this checks an *option on an alternative*,
// not the variance construct. Same reasoning that gave allocation_bound_read*
// its own pair in 047.
//
// Scaffolded ahead of 050/051. The baseline below is real and passes today —
// it is the unconstrained, unbounded alternative every TODO case contrasts
// against, so a regression in plain variance reading shows up here rather than
// being mistaken for an option bug.
//
// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.

#include <array>
#include <cstddef>
#include <string_view>
#include <variant>
#include <vector>
#include <ut>

#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

using plain =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::basic_field<"n", u32, 4_B>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0x01, s2s::as_trivial<u32, 4_B>>,
          s2s::match_case<0x02, s2s::as_vec<u8, s2s::len_from_field<"n">>>
        >
      >
    >
  >;

// A constraint on an as_vec alternative is checked against std::vector<u8>, the
// tag's resulting type, not against u8 — which is why it can be a predicate
// over the whole container. Spelled as a functor because the constraint is an
// NTTP and none of the built-in ones takes a vector value.
struct even_length {
  constexpr auto operator()(const std::vector<u8>& payload) const -> bool {
    return payload.size() % 2 == 0;
  }
};

// 0x03 declares nothing, so it is the control: the same byte pattern that
// violates 0x01's constraint is accepted here. It is a u16 rather than a second
// u32 because two alternatives of the same C++ type are rejected by
// has_unique_field_choices.
using constrained =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::basic_field<"n", u32, 4_B>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0x01, s2s::as_trivial<u32, 4_B, s2s::lte{99u}>>,
          s2s::match_case<0x02, s2s::as_vec<u8, s2s::len_from_field<"n">, even_length{}>>,
          s2s::match_case<0x03, s2s::as_trivial<u16, 2_B>>
        >
      >
    >
  >;

// max_bytes<8> over a u16 element admits four elements and no more; the string
// ceiling is in bytes outright. The record vector is bounded by sizeof(point),
// its in-memory footprint, which is unrelated to its two bytes on the wire.
// Each has an unbounded sibling of the same kind, so the ceiling under test is
// the declared one and not the default.
using point = s2s::struct_field_list<s2s::basic_field<"x", u16, 2_B>>;

using bounded =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::basic_field<"n", u32, 4_B>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0x01, s2s::as_vec<u16, s2s::len_from_field<"n">,
                                            s2s::max_bytes<8>>>,
          s2s::match_case<0x02, s2s::as_string<s2s::len_from_field<"n">,
                                               s2s::max_bytes<4>>>,
          s2s::match_case<0x03, s2s::as_vec_of_records<point, s2s::len_from_field<"n">,
                                                       s2s::max_bytes<64>>>,
          s2s::match_case<0x04, s2s::as_vec<u8, s2s::len_from_field<"n">>>
        >
      >
    >
  >;

// Ten bytes is not a whole number of u32s. The gate is an integer division, so
// the ceiling truncates down to two elements rather than admitting two and a
// half — a rounding direction worth pinning rather than inferring.
using ragged_bound =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::basic_field<"n", u32, 4_B>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0x01, s2s::as_vec<u32, s2s::len_from_field<"n">,
                                            s2s::max_bytes<10>>>
        >
      >
    >
  >;

// A bound beside a size and a constraint, in an order that puts none of the
// three where a positional reading would expect it.
using bounded_and_constrained =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::basic_field<"n", u32, 4_B>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0x01, s2s::as_vec<u8, even_length{}, s2s::max_bytes<8>,
                                            s2s::len_from_field<"n">>>
        >
      >
    >
  >;

// The same two-entry packs written the other way round.
using constrained_reversed =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::basic_field<"n", u32, 4_B>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0x01, s2s::as_trivial<u32, s2s::lte{99u}, 4_B>>,
          s2s::match_case<0x02, s2s::as_vec<u8, even_length{}, s2s::len_from_field<"n">>>,
          s2s::match_case<0x03, s2s::as_trivial<u16, 2_B>>
        >
      >
    >
  >;

// The read half of the interaction 052 calls out as most likely to be got
// wrong: 0x01 must be at most 99 *and* the resolved body must be even, and
// neither implies the other. Spelled as a functor because a std::variant is not
// a structural type and cannot be a template argument to eq{}.
struct even_body {
  constexpr auto operator()(const std::variant<u32, u16>& body) const -> bool {
    return std::visit([](auto value) { return value % 2 == 0; }, body);
  }
};

using both_constrained =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0x01, s2s::as_trivial<u32, 4_B, s2s::lte{99u}>>,
          s2s::match_case<0x02, s2s::as_trivial<u16, 2_B>>
        >
      >,
      even_body{}
    >
  >;

auto main() -> int {
  "an alternative with no options reads"_test = [] constexpr {
    std::array<u8, 12> buffer{0x01, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x00,
                              0x2a, 0x00, 0x00, 0x00};
    memstream<12> stream(buffer);

    auto res = s2s::struct_cast_le<plain>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::get<u32>((*res)["body"_f]), 42u));
  };

  "a container alternative with no options reads"_test = [] constexpr {
    std::array<u8, 11> buffer{0x02, 0x00, 0x00, 0x00,
                              0x03, 0x00, 0x00, 0x00,
                              0xaa, 0xbb, 0xcc};
    memstream<11> stream(buffer);

    auto res = s2s::struct_cast_le<plain>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::get<std::vector<u8>>((*res)["body"_f]).size(), std::size_t{3}));
  };

  "a satisfied alternative constraint reads normally"_test = [] constexpr {
    std::array<u8, 12> buffer{0x01, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x00,
                              0x2a, 0x00, 0x00, 0x00};
    memstream<12> stream(buffer);

    auto res = s2s::struct_cast_le<constrained>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::get<u32>((*res)["body"_f]), 42u));
  };

  // NON-VACUITY, READ SIDE. Deleting the check in read_variant_impl must turn
  // this red and leave the write-side file green — which is why the two live in
  // separate files rather than in one round-trip.
  //
  // failed_at is the union's id, not the alternative's: to_field_choices gives
  // every choice the union's id, and union_choice_pipeline_ct asserts that at
  // the type level.
  "a violated alternative constraint is rejected"_test = [] constexpr {
    std::array<u8, 12> buffer{0x01, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x00,
                              0xc8, 0x00, 0x00, 0x00};
    memstream<12> stream(buffer);

    auto res = s2s::struct_cast_le<constrained>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::validation_failure));
    expect(eq(res.error().failed_at, std::string_view{"body"}));
  };

  // The same 200 that alternative 0x01 refuses, through an alternative that
  // declared no constraint. A check leaking across alternatives would show up
  // here and nowhere else.
  "an alternative that declared no constraint is unaffected"_test = [] constexpr {
    std::array<u8, 10> buffer{0x03, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x00,
                              0xc8, 0x00};
    memstream<10> stream(buffer);

    auto res = s2s::struct_cast_le<constrained>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::get<u16>((*res)["body"_f]), u16{200}));
  };

  // The constraint applies to the tag's resulting type: even_length is a
  // predicate over std::vector<u8>, so it can see a length no per-element
  // predicate could.
  "a container alternative's constraint sees the whole container"_test = [] constexpr {
    std::array<u8, 11> buffer{0x02, 0x00, 0x00, 0x00,
                              0x03, 0x00, 0x00, 0x00,
                              0xaa, 0xbb, 0xcc};
    memstream<11> stream(buffer);

    auto res = s2s::struct_cast_le<constrained>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::validation_failure));
  };

  "pack entries are order-independent"_test = [] constexpr {
    std::array<u8, 12> buffer{0x01, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x00,
                              0xc8, 0x00, 0x00, 0x00};
    memstream<12> stream(buffer);

    auto res = s2s::struct_cast_le<constrained_reversed>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::validation_failure));
  };

  "a vector alternative under its declared bound is read"_test = [] constexpr {
    std::array<u8, 12> buffer{0x01, 0x00, 0x00, 0x00,
                              0x02, 0x00, 0x00, 0x00,
                              0x22, 0x11, 0x44, 0x33};
    memstream<12> stream(buffer);

    auto res = s2s::struct_cast_le<bounded>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::get<std::vector<u16>>((*res)["body"_f]).size(), std::size_t{2}));
  };

  // Inclusive, matching 047 exactly. Getting this backwards is the likeliest
  // silent error in the feature, which is why at-bound is its own case.
  "a vector alternative exactly at its declared bound is read"_test = [] constexpr {
    std::array<u8, 16> buffer{0x01, 0x00, 0x00, 0x00,
                              0x04, 0x00, 0x00, 0x00,
                              0x22, 0x11, 0x44, 0x33, 0x66, 0x55, 0x88, 0x77};
    memstream<16> stream(buffer);

    auto res = s2s::struct_cast_le<bounded>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::get<std::vector<u16>>((*res)["body"_f]).size(), std::size_t{4}));
  };

  // One element past the ceiling, with a stream long enough to satisfy it — so
  // the rejection is the bound talking, not the buffer running out.
  "a vector alternative over its declared bound is rejected"_test = [] constexpr {
    std::array<u8, 18> buffer{0x01, 0x00, 0x00, 0x00,
                              0x05, 0x00, 0x00, 0x00,
                              0x22, 0x11, 0x44, 0x33, 0x66, 0x55, 0x88, 0x77, 0x00, 0x00};
    memstream<18> stream(buffer);

    auto res = s2s::struct_cast_le<bounded>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::excessive_length));
    expect(eq(res.error().failed_at, std::string_view{"body"}));
  };

  "a string alternative is bounded by the same ceiling"_test = [] constexpr {
    std::array<u8, 12> buffer{0x02, 0x00, 0x00, 0x00,
                              0x04, 0x00, 0x00, 0x00,
                              'h', 'e', 'l', 'l'};
    memstream<12> at_bound(buffer);
    expect(eq(s2s::struct_cast_le<bounded>(at_bound).has_value(), true));

    std::array<u8, 13> longer{0x02, 0x00, 0x00, 0x00,
                              0x05, 0x00, 0x00, 0x00,
                              'h', 'e', 'l', 'l', 'o'};
    memstream<13> over_bound(longer);
    auto res = s2s::struct_cast_le<bounded>(over_bound);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::excessive_length));
  };

  // The denominator is the record's in-memory footprint, which has no
  // relationship to how many bytes those records occupy on the wire.
  "a record vector alternative is bounded by its footprint"_test = [] constexpr {
    std::array<u8, 10> buffer{0x03, 0x00, 0x00, 0x00,
                              0x01, 0x00, 0x00, 0x00,
                              0x22, 0x11};
    memstream<10> stream(buffer);
    expect(eq(s2s::struct_cast_le<bounded>(stream).has_value(), true));

    std::array<u8, 10> absurd{0x03, 0x00, 0x00, 0x00,
                              0xff, 0xff, 0x00, 0x00,
                              0x00, 0x00};
    memstream<10> absurd_stream(absurd);
    auto res = s2s::struct_cast_le<bounded>(absurd_stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::excessive_length));
  };

  // Omission is not "unbounded": the sibling that declared nothing still has
  // the default standing behind it. 0x01000000 u8 elements is 16 MiB + 0, one
  // past the shipped default.
  "an alternative declaring no bound still gets the default"_test = [] constexpr {
    std::array<u8, 8> buffer{0x04, 0x00, 0x00, 0x00,
                             0x01, 0x00, 0x00, 0x01};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<bounded>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::excessive_length));
  };

  "a bound that is not a multiple of the element size truncates down"_test = [] constexpr {
    std::array<u8, 16> buffer{0x01, 0x00, 0x00, 0x00,
                              0x02, 0x00, 0x00, 0x00,
                              0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe, 0xca};
    memstream<16> stream(buffer);
    expect(eq(s2s::struct_cast_le<ragged_bound>(stream).has_value(), true));

    std::array<u8, 20> wider{0x01, 0x00, 0x00, 0x00,
                             0x03, 0x00, 0x00, 0x00,
                             0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe, 0xca,
                             0x00, 0x00, 0x00, 0x00};
    memstream<20> wider_stream(wider);
    auto res = s2s::struct_cast_le<ragged_bound>(wider_stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::excessive_length));
  };

  // All three kinds of entry in one pack, in an order no positional reading
  // would produce. Each still has to bite on its own terms.
  "a bound composes with a size and a constraint in any order"_test = [] constexpr {
    std::array<u8, 12> buffer{0x01, 0x00, 0x00, 0x00,
                              0x04, 0x00, 0x00, 0x00,
                              0xaa, 0xbb, 0xcc, 0xdd};
    memstream<12> stream(buffer);
    expect(eq(s2s::struct_cast_le<bounded_and_constrained>(stream).has_value(), true));

    std::array<u8, 11> odd{0x01, 0x00, 0x00, 0x00,
                           0x03, 0x00, 0x00, 0x00,
                           0xaa, 0xbb, 0xcc};
    memstream<11> odd_stream(odd);
    auto violates_constraint = s2s::struct_cast_le<bounded_and_constrained>(odd_stream);
    expect(eq(violates_constraint.has_value(), false));
    expect(eq(violates_constraint.error().failure_reason,
              s2s::error_reason::validation_failure));

    std::array<u8, 18> over{0x01, 0x00, 0x00, 0x00,
                            0x0a, 0x00, 0x00, 0x00,
                            0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11, 0x22, 0x33};
    memstream<18> over_stream(over);
    auto violates_bound = s2s::struct_cast_le<bounded_and_constrained>(over_stream);
    expect(eq(violates_bound.has_value(), false));
    expect(eq(violates_bound.error().failure_reason,
              s2s::error_reason::excessive_length));
  };

  "both constraints hold and the read succeeds"_test = [] constexpr {
    std::array<u8, 8> buffer{0x01, 0x00, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x00};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<both_constrained>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::get<u32>((*res)["body"_f]), 42u));
  };

  // The two rejections below are indistinguishable by their error — every
  // alternative takes the union's id, so both report validation_failure at
  // "body". What separates them is the value: 43 satisfies lte{99u} and only
  // the union constraint can reject it, 200 is even and only the alternative's
  // can. Each therefore proves one check fired with the other passing.
  "the union constraint rejects a value its alternative accepts"_test = [] constexpr {
    std::array<u8, 8> buffer{0x01, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x00, 0x00};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<both_constrained>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::validation_failure));
  };

  "the alternative constraint rejects a value the union accepts"_test = [] constexpr {
    std::array<u8, 8> buffer{0x01, 0x00, 0x00, 0x00, 0xc8, 0x00, 0x00, 0x00};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<both_constrained>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::validation_failure));
  };
}
