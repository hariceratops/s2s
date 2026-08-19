// A constraint declared on the variance itself, write path, compile-time tier.
// Companion to union_constraint_read_ct.cpp; see its header for why this is its
// own pair rather than an addition to union_alternative_options_*.
//
// A separate file from the read one for the project's usual reason, not 050's:
// this constraint has no dedicated call site to prove non-vacuous in either
// direction. It runs at the struct-level fold, and on the write side that fold
// validates a field before writing it, so a rejected variant contributes no
// bytes of its own.
//
// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.

#include <array>
#include <cstddef>
#include <string_view>
#include <variant>
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

struct even_body {
  constexpr auto operator()(const std::variant<u32, u16>& body) const -> bool {
    return std::visit([](auto value) { return value % 2 == 0; }, body);
  }
};

using plain =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0x01, s2s::as_trivial<u32, 4_B>>,
          s2s::match_case<0x02, s2s::as_trivial<u16, 2_B>>
        >
      >
    >
  >;

using constrained =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0x01, s2s::as_trivial<u32, 4_B>>,
          s2s::match_case<0x02, s2s::as_trivial<u16, 2_B>>
        >
      >,
      even_body{}
    >
  >;

auto main() -> int {
  // Only "body" is assigned: "tag" is the discriminant, derived from the held
  // alternative rather than stored, and 043 made it non-assignable.
  "a variance declaring no constraint writes as before"_test = [] constexpr {
    plain obj{};
    obj["body"_f] = 43u;

    std::array<u8, 8> buffer{};
    memstream<8> stream(buffer);

    auto res = s2s::stream_cast_le<plain>(stream, obj);

    expect(eq(res.has_value(), true));
    expect(eq(buffer[4], u8{0x2b}));
  };

  "a resolved variant satisfying the union constraint writes"_test = [] constexpr {
    constrained obj{};
    obj["body"_f] = 42u;

    std::array<u8, 8> buffer{};
    memstream<8> stream(buffer);

    auto res = s2s::stream_cast_le<constrained>(stream, obj);

    expect(eq(res.has_value(), true));
    expect(eq(buffer[0], u8{0x01}));
    expect(eq(buffer[4], u8{0x2a}));
  };

  // NON-VACUITY. The same value writes fine through `plain`.
  //
  // The check runs at the fold, which validates a field before writing it, so
  // the union contributed no bytes: what reached the stream is the discriminant
  // ahead of it, written when the fold reached "tag", and nothing more.
  "a resolved variant violating the union constraint contributes no bytes"_test =
    [] constexpr {
      constrained obj{};
      obj["body"_f] = 43u;

      std::array<u8, 8> buffer{};
      memstream<8> stream(buffer);

      auto res = s2s::stream_cast_le<constrained>(stream, obj);

      expect(eq(res.has_value(), false));
      expect(eq(res.error().failure_reason, s2s::error_reason::validation_failure));
      expect(eq(res.error().failed_at, std::string_view{"body"}));
      expect(eq(buffer[0], u8{0x01}));
      expect(eq(buffer[4], u8{0x00}));
    };

  "the union constraint applies to every alternative"_test = [] constexpr {
    constrained accepted_obj{};
    accepted_obj["body"_f] = u16{200};

    std::array<u8, 6> accepted{};
    memstream<6> accepted_stream(accepted);

    auto accepted_res = s2s::stream_cast_le<constrained>(accepted_stream, accepted_obj);

    expect(eq(accepted_res.has_value(), true));
    expect(eq(accepted[4], u8{0xc8}));

    constrained rejected_obj{};
    rejected_obj["body"_f] = u16{201};

    std::array<u8, 6> rejected{};
    memstream<6> rejected_stream(rejected);

    auto rejected_res = s2s::stream_cast_le<constrained>(rejected_stream, rejected_obj);

    expect(eq(rejected_res.has_value(), false));
    expect(eq(rejected_res.error().failure_reason, s2s::error_reason::validation_failure));
    expect(eq(rejected_res.error().failed_at, std::string_view{"body"}));
  };
}
