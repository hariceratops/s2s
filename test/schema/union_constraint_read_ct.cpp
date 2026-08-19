// A constraint declared on the variance itself, read path, compile-time tier.
//
// Its own pair rather than additions to union_alternative_options_read*,
// because the tree is organised by what a file verifies and these two
// constraints are different things: an alternative's constrains one
// alternative's payload, this one constrains the resolved variant and sees
// every alternative. The case where both are declared lives in
// union_alternative_options_write*, since there it is the combination under
// test.
//
// Unlike the per-alternative constraint, this one needs no new call site: a
// union_field derives from field, so the struct-level fold in struct_cast_impl
// runs it like any other field's. That is what these tests are really pinning.
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

// Spelled as a functor, not as one of the built-in comparisons: the value being
// constrained is a std::variant, which is not a structural type and so cannot
// be a template argument to eq{} at all.
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
  "a variance declaring no constraint reads as before"_test = [] constexpr {
    std::array<u8, 8> buffer{0x01, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x00, 0x00};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<plain>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::get<u32>((*res)["body"_f]), 43u));
  };

  "a resolved variant satisfying the union constraint reads"_test = [] constexpr {
    std::array<u8, 8> buffer{0x01, 0x00, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x00};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<constrained>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::get<u32>((*res)["body"_f]), 42u));
  };

  // NON-VACUITY. The same bytes read fine through `plain`, so the rejection is
  // the constraint and nothing else.
  "a resolved variant violating the union constraint is rejected"_test = [] constexpr {
    std::array<u8, 8> buffer{0x01, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x00, 0x00};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<constrained>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::validation_failure));
    expect(eq(res.error().failed_at, std::string_view{"body"}));
  };

  // The point of a union-level constraint: it holds whichever alternative was
  // resolved, and neither alternative here declares one of its own.
  "the union constraint applies to every alternative"_test = [] constexpr {
    std::array<u8, 6> accepted{0x02, 0x00, 0x00, 0x00, 0xc8, 0x00};
    memstream<6> accepted_stream(accepted);

    auto accepted_res = s2s::struct_cast_le<constrained>(accepted_stream);

    expect(eq(accepted_res.has_value(), true));
    expect(eq(std::get<u16>((*accepted_res)["body"_f]), u16{200}));

    std::array<u8, 6> rejected{0x02, 0x00, 0x00, 0x00, 0xc9, 0x00};
    memstream<6> rejected_stream(rejected);

    auto rejected_res = s2s::struct_cast_le<constrained>(rejected_stream);

    expect(eq(rejected_res.has_value(), false));
    expect(eq(rejected_res.error().failure_reason, s2s::error_reason::validation_failure));
    expect(eq(rejected_res.error().failed_at, std::string_view{"body"}));
  };
}
