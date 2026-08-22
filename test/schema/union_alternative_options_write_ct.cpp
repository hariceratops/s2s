// Options declared on a union alternative, write path, compile-time tier.
// Companion to union_alternative_options_read_ct.cpp; see its header for why
// this is its own pair rather than an addition to union_write*.
//
// The write side is a separate file, not a separate test in the read file, for
// the reason 050 turns on: the per-alternative constraint needs an explicit
// check at write_variant_impl *and* one at read_variant_impl, and a single test
// exercising a round-trip would stay green with either one missing.
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

// 0x02 declares no constraint and is the control the write side needs just as
// the read side does. No container alternative here: the constrained-container
// case needs a sibling length assigned alongside the payload, which reads
// better in the run-time file.
using constrained =
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
      >
    >
  >;

// The interaction 052 calls out as the one most likely to be got wrong. Same
// alternatives as `constrained`, with a constraint on the variance as well:
// 0x01 must be at most 99 *and* the resolved body must be even, and neither
// implies the other. Spelled as a functor because a std::variant is not a
// structural type and cannot be a template argument to eq{}.
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
  // Only "body" is assigned: "tag" is the discriminant and "n" the length
  // target, both derived from the held alternative rather than stored, and 043
  // made them non-assignable so a schema author cannot contradict the
  // derivation.
  "an alternative with no options writes"_test = [] constexpr {
    plain obj{};
    obj["body"_f] = 42u;

    std::array<u8, 12> buffer{};
    memstream<12> stream(buffer);

    auto res = s2s::stream_cast_le<plain>(stream, obj);

    expect(eq(res.has_value(), true));
    expect(eq(buffer[0], u8{0x01}));
    expect(eq(buffer[8], u8{0x2a}));
  };

  "a satisfied alternative constraint writes normally"_test = [] constexpr {
    constrained obj{};
    obj["body"_f] = 42u;

    std::array<u8, 8> buffer{};
    memstream<8> stream(buffer);

    auto res = s2s::stream_cast_le<constrained>(stream, obj);

    expect(eq(res.has_value(), true));
    expect(eq(buffer[0], u8{0x01}));
    expect(eq(buffer[4], u8{0x2a}));
  };

  // NON-VACUITY, WRITE SIDE. Deleting the check in write_variant_impl must turn
  // this red and leave union_alternative_options_read_ct green.
  //
  // The payload bytes stay zero because the check runs before write_field: a
  // discarded value is recoverable, half a record on a stream is not.
  "a violated alternative constraint is rejected before any payload is written"_test =
    [] constexpr {
      constrained obj{};
      obj["body"_f] = 200u;

      std::array<u8, 8> buffer{};
      memstream<8> stream(buffer);

      auto res = s2s::stream_cast_le<constrained>(stream, obj);

      expect(eq(res.has_value(), false));
      expect(eq(res.error().failure_reason, s2s::error_reason::validation_failure));
      expect(eq(res.error().failed_at, std::string_view{"body"}));
      expect(eq(buffer[4], u8{0x00}));
    };

  "an alternative that declared no constraint is unaffected"_test = [] constexpr {
    constrained obj{};
    obj["body"_f] = u16{200};

    std::array<u8, 6> buffer{};
    memstream<6> stream(buffer);

    auto res = s2s::stream_cast_le<constrained>(stream, obj);

    expect(eq(res.has_value(), true));
    expect(eq(buffer[4], u8{0xc8}));
  };

  "both constraints hold and the write succeeds"_test = [] constexpr {
    both_constrained obj{};
    obj["body"_f] = 42u;

    std::array<u8, 8> buffer{};
    memstream<8> stream(buffer);

    auto res = s2s::stream_cast_le<both_constrained>(stream, obj);

    expect(eq(res.has_value(), true));
    expect(eq(buffer[4], u8{0x2a}));
  };

  // The two rejections below are indistinguishable by their error — every
  // alternative takes the union's id, so both report validation_failure at
  // "body". What separates them is the value: 43 satisfies lte{99u} and only
  // the union constraint can reject it, 200 is even and only the alternative's
  // can. Each therefore proves one check fired with the other passing.
  "the union constraint rejects a value its alternative accepts"_test = [] constexpr {
    both_constrained obj{};
    obj["body"_f] = 43u;

    std::array<u8, 8> buffer{};
    memstream<8> stream(buffer);

    auto res = s2s::stream_cast_le<both_constrained>(stream, obj);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::validation_failure));
  };

  "the alternative constraint rejects a value the union accepts"_test = [] constexpr {
    both_constrained obj{};
    obj["body"_f] = 200u;

    std::array<u8, 8> buffer{};
    memstream<8> stream(buffer);

    auto res = s2s::stream_cast_le<both_constrained>(stream, obj);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::validation_failure));
  };

  //
  // Note for 051: there is deliberately no write-side bound case. A bound
  // guards an allocation driven by a wire length; the write path serialises a
  // container the caller already owns, so there is nothing untrusted to bound.
  // If a write-side bound case ever seems necessary, the spec's Non-Goals
  // ("write path untouched") is the thing to re-read first.
}
