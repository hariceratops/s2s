// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Every s2s constraint below is written
// s2s-qualified and every ut matcher is ut::eq. Test lambdas must not capture:
// ut skips a capturing lambda at compile time silently.

#include <array>
#include <cstddef>
#include <string_view>
#include <ut>

#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

using u32 = unsigned int;

template <auto constraint>
using constrained =
  s2s::struct_field_list<
    s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>, constraint>
  >;

template <auto constraint>
constexpr auto write(u32 value) -> s2s::cast_result {
  std::array<u8, 4> buffer{};
  memstream<4> stream(buffer);
  constrained<constraint> obj{};
  obj["a"_f] = value;
  return s2s::stream_cast_le<constrained<constraint>>(stream, obj);
}

int main() {
  // The read side checks after taking bytes off the wire; the write side has
  // to check before putting any on. Same forms, opposite order.
  "a write consults eq"_test = [] constexpr {
    constexpr auto c = s2s::eq(42u);
    expect(eq(write<c>(42u).has_value(), true));
    expect(eq(write<c>(43u).has_value(), false));
  };

  "a write consults neq"_test = [] constexpr {
    constexpr auto c = s2s::neq(42u);
    expect(eq(write<c>(43u).has_value(), true));
    expect(eq(write<c>(42u).has_value(), false));
  };

  "a write consults the ordering forms at their boundaries"_test = [] constexpr {
    constexpr auto less = s2s::lt(42u);
    expect(eq(write<less>(41u).has_value(), true));
    expect(eq(write<less>(42u).has_value(), false));

    constexpr auto greater = s2s::gt(42u);
    expect(eq(write<greater>(43u).has_value(), true));
    expect(eq(write<greater>(42u).has_value(), false));

    constexpr auto at_most = s2s::lte(42u);
    expect(eq(write<at_most>(42u).has_value(), true));
    expect(eq(write<at_most>(43u).has_value(), false));

    constexpr auto at_least = s2s::gte(42u);
    expect(eq(write<at_least>(42u).has_value(), true));
    expect(eq(write<at_least>(41u).has_value(), false));
  };

  "a write consults any_of"_test = [] constexpr {
    constexpr auto c = s2s::any_of(21u, 42u, 84u);
    expect(eq(write<c>(84u).has_value(), true));
    expect(eq(write<c>(43u).has_value(), false));
  };

  "no_constraint lets any value through"_test = [] constexpr {
    constexpr auto none = s2s::no_constraint<u32>{};
    expect(eq(write<none>(0u).has_value(), true));
    expect(eq(write<none>(0xdeadbeefu).has_value(), true));
  };

  "a rejected value names its field"_test = [] constexpr {
    constexpr auto c = s2s::lt(100u);
    auto written = write<c>(500u);

    expect(eq(written.has_value(), false));
    expect(eq(written.error().failure_reason, s2s::error_reason::validation_failure));
    expect(eq(written.error().failed_at, std::string_view{"a"}));
  };

  // Fail-fast, not rollback: the field ahead of the violation stays on the
  // stream, and the offending field contributes nothing.
  "the offending field contributes no bytes"_test = [] constexpr {
    using two =
      s2s::struct_field_list<
        s2s::basic_field<"lead", u32, s2s::field_size<s2s::fixed<4>>>,
        s2s::basic_field<"bounded", u32, s2s::field_size<s2s::fixed<4>>, s2s::lt(100u)>
      >;

    std::array<u8, 8> buffer{};
    memstream<8> stream(buffer);
    two obj{};
    obj["lead"_f] = 0x11223344;
    obj["bounded"_f] = 500;

    auto written = s2s::stream_cast_le<two>(stream, obj);

    expect(eq(written.has_value(), false));
    expect(eq(written.error().failed_at, std::string_view{"bounded"}));
    constexpr std::array<u8, 8> expected{0x44, 0x33, 0x22, 0x11, 0x00, 0x00, 0x00, 0x00};
    for(std::size_t idx = 0; idx < expected.size(); ++idx)
      expect(eq(buffer[idx], expected[idx]));
  };
}
