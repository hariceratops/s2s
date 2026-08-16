// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.
//
// The predicate and compute lambdas are namespace scope on purpose. They are
// NTTPs of the schema, not captures of the test body.

#include <array>
#include <string_view>
#include <variant>
#include <ut>

#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

using u32 = unsigned int;

using inner_1 =
  s2s::struct_field_list<
    s2s::basic_field<"x", u32, 4_B>,
    s2s::basic_field<"y", u32, 4_B>
  >;
using inner_2 =
  s2s::struct_field_list<
    s2s::basic_field<"p", u32, 4_B>,
    s2s::basic_field<"q", u32, 4_B>
  >;

// Form 1: switch on a discriminant field.
using switched =
  s2s::struct_field_list<
    s2s::basic_field<"a", u32, 4_B>,
    s2s::basic_field<"b", u32, 4_B>,
    s2s::variance<
      "c",
      s2s::type<
        s2s::match_field<"a">,
        s2s::type_switch<
          s2s::match_case<0xcafed00d, s2s::as_struct<inner_1>>,
          s2s::match_case<0xdeadbeef, s2s::as_struct<inner_2>>
        >
      >
    >
  >;

// The two record tags 048 makes nameable. Their sizes work the way the
// non-record forms do: an array's extent is a template parameter, a vector's
// comes from a sibling length field.
using with_record_arr =
  s2s::struct_field_list<
    s2s::basic_field<"a", u32, 4_B>,
    s2s::variance<
      "c",
      s2s::type<
        s2s::match_field<"a">,
        s2s::type_switch<
          s2s::match_case<0xcafed00d, s2s::as_arr_of_records<inner_1, 2>>
        >
      >
    >
  >;

using with_record_vec =
  s2s::struct_field_list<
    s2s::basic_field<"a", u32, 4_B>,
    s2s::basic_field<"n", u32, 4_B>,
    s2s::variance<
      "c",
      s2s::type<
        s2s::match_field<"a">,
        s2s::type_switch<
          s2s::match_case<0xcafed00d, s2s::as_vec_of_records<inner_1, s2s::len_from_field<"n">>>
        >
      >
    >
  >;

// Form 2: a ladder of arbitrary predicates.
auto sum_in_low_band = [](auto a, auto b) { return a + b >= 20000 && a + b < 40000; };
auto sum_in_high_band = [](auto a, auto b) { return a + b >= 40000 && a + b < 60000; };

using laddered =
  s2s::struct_field_list<
    s2s::basic_field<"a", u32, 4_B>,
    s2s::basic_field<"b", u32, 4_B>,
    s2s::variance<
      "c",
      s2s::type<
        s2s::type_if_else<
          s2s::branch<
            s2s::predicate<sum_in_low_band, "a", "b">,
            s2s::as_trivial<u32, 4_B>
          >,
          s2s::branch<
            s2s::predicate<sum_in_high_band, "a", "b">,
            s2s::as_struct<inner_1>
          >
        >
      >
    >
  >;

// Form 3: switch on a value computed from several fields.
auto sum_of = [](auto a, auto b) { return a + b; };

using computed =
  s2s::struct_field_list<
    s2s::basic_field<"a", u32, 4_B>,
    s2s::basic_field<"b", u32, 4_B>,
    s2s::variance<
      "c",
      s2s::type<
        s2s::compute<sum_of, u32, "a", "b">,
        s2s::type_switch<
          s2s::match_case<200, s2s::as_trivial<u32, 4_B>>,
          s2s::match_case<300, s2s::as_struct<inner_1>>
        >
      >
    >
  >;

auto main() -> int {
  "a discriminant switch selects its matching alternative"_test = [] constexpr {
    std::array<u8, 16> buffer{
      0xef, 0xbe, 0xad, 0xde,
      0x0d, 0xd0, 0xfe, 0xca,
      0xef, 0xbe, 0xad, 0xde,
      0x0d, 0xd0, 0xfe, 0xca
    };
    memstream<16> stream(buffer);

    auto res = s2s::struct_cast_le<switched>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["a"_f], 0xdeadbeefu));
    expect(eq((*res)["b"_f], 0xcafed00du));
    // "a" is 0xdeadbeef, so the second match_case wins.
    expect(eq(std::holds_alternative<inner_2>((*res)["c"_f]), true));
    expect(eq(std::get<inner_2>((*res)["c"_f])["p"_f], 0xdeadbeefu));
    expect(eq(std::get<inner_2>((*res)["c"_f])["q"_f], 0xcafed00du));
  };

  // The other match_case, so the switch is shown choosing rather than always
  // landing on the same alternative.
  "a different discriminant selects the other alternative"_test = [] constexpr {
    std::array<u8, 16> buffer{
      0x0d, 0xd0, 0xfe, 0xca,
      0xef, 0xbe, 0xad, 0xde,
      0xef, 0xbe, 0xef, 0xbe,
      0x0d, 0xd0, 0xfe, 0xca
    };
    memstream<16> stream(buffer);

    auto res = s2s::struct_cast_le<switched>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::holds_alternative<inner_1>((*res)["c"_f]), true));
    expect(eq(std::get<inner_1>((*res)["c"_f])["x"_f], 0xbeefbeefu));
    expect(eq(std::get<inner_1>((*res)["c"_f])["y"_f], 0xcafed00du));
  };

  "a discriminant no case matches fails type deduction"_test = [] constexpr {
    std::array<u8, 16> buffer{
      0x11, 0x22, 0x33, 0x44,
      0x0d, 0xd0, 0xfe, 0xca,
      0xef, 0xbe, 0xad, 0xde,
      0x0d, 0xd0, 0xfe, 0xca
    };
    memstream<16> stream(buffer);

    auto res = s2s::struct_cast_le<switched>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::type_deduction_failure));
    expect(eq(res.error().failed_at, std::string_view{"c"}));
  };

  // Alternatives are read at the declared byte order like any other field.
  "a selected alternative takes the declared byte order"_test = [] constexpr {
    std::array<u8, 16> buffer{
      0xde, 0xad, 0xbe, 0xef,
      0xca, 0xfe, 0xd0, 0x0d,
      0xbe, 0xef, 0xbe, 0xef,
      0xca, 0xfe, 0xd0, 0x0d
    };
    memstream<16> stream(buffer);

    auto res = s2s::struct_cast_be<switched>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::get<inner_2>((*res)["c"_f])["p"_f], 0xbeefbeefu));
    expect(eq(std::get<inner_2>((*res)["c"_f])["q"_f], 0xcafed00du));
  };

  "a predicate ladder selects the first branch that holds"_test = [] constexpr {
    // 12000 + 12000 = 24000, inside the low band.
    std::array<u8, 12> buffer{
      0xe0, 0x2e, 0x00, 0x00,
      0xe0, 0x2e, 0x00, 0x00,
      0x0d, 0xd0, 0xfe, 0xca
    };
    memstream<12> stream(buffer);

    auto res = s2s::struct_cast_le<laddered>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::holds_alternative<u32>((*res)["c"_f]), true));
    expect(eq(std::get<u32>((*res)["c"_f]), 0xcafed00du));
  };

  "a later ladder branch is selected when the earlier one fails"_test = [] constexpr {
    // 25000 + 25000 = 50000, inside the high band, whose alternative is a
    // record — so a ladder has to be able to select a nested schema too.
    std::array<u8, 16> buffer{
      0xa8, 0x61, 0x00, 0x00,
      0xa8, 0x61, 0x00, 0x00,
      0xef, 0xbe, 0xef, 0xbe,
      0x0d, 0xd0, 0xfe, 0xca
    };
    memstream<16> stream(buffer);

    auto res = s2s::struct_cast_le<laddered>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::holds_alternative<inner_1>((*res)["c"_f]), true));
    expect(eq(std::get<inner_1>((*res)["c"_f])["x"_f], 0xbeefbeefu));
  };

  "a ladder where no branch holds fails type deduction"_test = [] constexpr {
    // 100 + 100 = 200, below every band.
    std::array<u8, 12> buffer{
      0x64, 0x00, 0x00, 0x00,
      0x64, 0x00, 0x00, 0x00,
      0x0d, 0xd0, 0xfe, 0xca
    };
    memstream<12> stream(buffer);

    auto res = s2s::struct_cast_le<laddered>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::type_deduction_failure));
    expect(eq(res.error().failed_at, std::string_view{"c"}));
  };

  "a computed switch matches on the computed value"_test = [] constexpr {
    // 100 + 100 = 200, so the trivial alternative wins — the value is not on
    // the wire anywhere.
    std::array<u8, 12> buffer{
      0x64, 0x00, 0x00, 0x00,
      0x64, 0x00, 0x00, 0x00,
      0xef, 0xbe, 0xef, 0xbe
    };
    memstream<12> stream(buffer);

    auto res = s2s::struct_cast_le<computed>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::holds_alternative<u32>((*res)["c"_f]), true));
    expect(eq(std::get<u32>((*res)["c"_f]), 0xbeefbeefu));
  };

  "a computed switch with no matching case fails type deduction"_test = [] constexpr {
    // 100 + 1 = 101, which is neither 200 nor 300.
    std::array<u8, 12> buffer{
      0x64, 0x00, 0x00, 0x00,
      0x01, 0x00, 0x00, 0x00,
      0xef, 0xbe, 0xef, 0xbe
    };
    memstream<12> stream(buffer);

    auto res = s2s::struct_cast_le<computed>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::type_deduction_failure));
    expect(eq(res.error().failed_at, std::string_view{"c"}));
  };

  // The first coverage as_arr_of_records and as_vec_of_records have ever had.
  // Both were documented but had no is_type_tag specialization, so match_case
  // rejected them and no schema could name one; 048 admits them. Admitting a
  // tag is not the same as its working, which is what these two check.
  "an array-of-records alternative reads"_test = [] constexpr {
    std::array<u8, 20> buffer{
      0x0d, 0xd0, 0xfe, 0xca,
      0x11, 0x11, 0x11, 0x11, 0x22, 0x22, 0x22, 0x22,
      0x33, 0x33, 0x33, 0x33, 0x44, 0x44, 0x44, 0x44
    };
    memstream<20> stream(buffer);

    auto res = s2s::struct_cast_le<with_record_arr>(stream);

    expect(eq(res.has_value(), true));
    auto& held = std::get<std::array<inner_1, 2>>((*res)["c"_f]);
    expect(eq(held[0]["x"_f], 0x11111111u));
    expect(eq(held[1]["y"_f], 0x44444444u));
  };

  "a vector-of-records alternative reads"_test = [] constexpr {
    // Two inner_1 records of eight bytes each, after the four-byte
    // discriminant and the four-byte length.
    std::array<u8, 24> buffer{
      0x0d, 0xd0, 0xfe, 0xca,
      0x02, 0x00, 0x00, 0x00,
      0x11, 0x11, 0x11, 0x11, 0x22, 0x22, 0x22, 0x22,
      0x33, 0x33, 0x33, 0x33, 0x44, 0x44, 0x44, 0x44
    };
    memstream<24> stream(buffer);

    auto res = s2s::struct_cast_le<with_record_vec>(stream);

    expect(eq(res.has_value(), true));
    auto& held = std::get<std::vector<inner_1>>((*res)["c"_f]);
    expect(eq(held.size(), std::size_t{2}));
    expect(eq(held[0]["x"_f], 0x11111111u));
  };
}
