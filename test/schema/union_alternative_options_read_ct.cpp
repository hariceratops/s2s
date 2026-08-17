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

  //
  // TODO(051): below-bound accepted, at-bound accepted (inclusive), over-bound
  // rejected with error_reason::excessive_length, for as_vec, as_string and
  // as_vec_of_records alternatives.
  //
  // TODO(051): a bound that is not a whole multiple of sizeof(element) — the
  // gate is phrased as an integer division, so the rounding direction is
  // observable and has to be pinned.
  //
  // TODO(051): a declared alternative bound is not clamped by
  // S2S_DEFAULT_MAX_BYTES in either direction. Needs its own translation unit
  // to vary the macro; allocation_bound_default_override_ct.cpp and
  // allocation_bound_disabled_ct.cpp are the models.
}
