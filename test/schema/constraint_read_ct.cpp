// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. That collision is the whole subject
// of this file, so every s2s constraint below is written s2s-qualified and
// every ut matcher is ut::eq. Test lambdas must not capture: ut skips a
// capturing lambda at compile time silently.

#include <array>
#include <expected>
#include <string_view>
#include <ut>

#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

using u32 = unsigned int;

// One constrained field, read from four little-endian bytes. Every form below
// is exercised the same way, so a difference in outcome is the constraint's.
template <auto constraint>
using constrained =
  s2s::struct_field_list<
    s2s::basic_field<"a", u32, 4_B, constraint>
  >;

template <auto constraint>
constexpr auto read(u32 value) -> std::expected<constrained<constraint>, s2s::cast_error> {
  std::array<u8, 4> buffer{
    static_cast<u8>(value & 0xff),
    static_cast<u8>((value >> 8) & 0xff),
    static_cast<u8>((value >> 16) & 0xff),
    static_cast<u8>((value >> 24) & 0xff)
  };
  memstream<4> stream(buffer);
  return s2s::struct_cast_le<constrained<constraint>>(stream);
}

// A violated constraint must fail with this reason and name this field, in
// every form. Asserted per form rather than once, since each reaches the
// check by its own predicate.
constexpr auto rejected_naming_a(const auto& result) -> bool {
  return !result.has_value() &&
         result.error().failure_reason == s2s::error_reason::validation_failure &&
         result.error().failed_at == std::string_view{"a"};
}

auto main() -> int {
  // --- the constraint objects, called directly -------------------------
  // Ported from test/runtime/field_validators.cpp unchanged. These check the
  // predicates themselves; the group below checks them reaching a read.

  "eq accepts its value and rejects others"_test = [] constexpr {
    constexpr auto obj = s2s::eq(42);
    expect(eq(obj(42), true));
    expect(eq(obj(84), false));
  };

  "neq rejects its value and accepts others"_test = [] constexpr {
    constexpr auto obj = s2s::neq(42);
    expect(eq(obj(42), false));
    expect(eq(obj(84), true));
  };

  "lt accepts only smaller values"_test = [] constexpr {
    constexpr auto obj = s2s::lt(42);
    expect(eq(obj(21), true));
    expect(eq(obj(84), false));
  };

  "gt accepts only larger values"_test = [] constexpr {
    constexpr auto obj = s2s::gt(42);
    expect(eq(obj(21), false));
    expect(eq(obj(84), true));
  };

  "lte accepts smaller values and the boundary"_test = [] constexpr {
    constexpr auto obj = s2s::lte(42);
    expect(eq(obj(21), true));
    expect(eq(obj(42), true));
    expect(eq(obj(84), false));
  };

  "gte accepts larger values and the boundary"_test = [] constexpr {
    constexpr auto obj = s2s::gte(42);
    expect(eq(obj(21), false));
    expect(eq(obj(42), true));
    expect(eq(obj(84), true));
  };

  "any_of accepts each listed value and nothing else"_test = [] constexpr {
    constexpr auto obj = s2s::any_of(21, 42, 84);
    expect(eq(obj(21), true));
    expect(eq(obj(42), true));
    expect(eq(obj(84), true));
    expect(eq(obj(100), false));
  };

  // --- the same forms, reaching an actual read -------------------------
  // A predicate that returns the right answer is not the same as a read that
  // consults it. Each form is read once satisfied and once violated.

  "no_constraint accepts whatever is on the wire"_test = [] constexpr {
    constexpr auto none = s2s::no_constraint<u32>{};
    expect(eq(read<none>(0u).has_value(), true));
    expect(eq(read<none>(0xdeadbeefu).has_value(), true));
    expect(eq((*read<none>(0xdeadbeefu))["a"_f], 0xdeadbeefu));
  };

  "a read consults eq"_test = [] constexpr {
    constexpr auto c = s2s::eq(42u);
    expect(eq(read<c>(42u).has_value(), true));
    expect(eq(rejected_naming_a(read<c>(43u)), true));
  };

  "a read consults neq"_test = [] constexpr {
    constexpr auto c = s2s::neq(42u);
    expect(eq(read<c>(43u).has_value(), true));
    expect(eq(rejected_naming_a(read<c>(42u)), true));
  };

  "a read consults lt"_test = [] constexpr {
    constexpr auto c = s2s::lt(42u);
    expect(eq(read<c>(41u).has_value(), true));
    // The boundary is the interesting side: lt must reject its own value.
    expect(eq(rejected_naming_a(read<c>(42u)), true));
  };

  "a read consults gt"_test = [] constexpr {
    constexpr auto c = s2s::gt(42u);
    expect(eq(read<c>(43u).has_value(), true));
    expect(eq(rejected_naming_a(read<c>(42u)), true));
  };

  "a read consults lte"_test = [] constexpr {
    constexpr auto c = s2s::lte(42u);
    expect(eq(read<c>(42u).has_value(), true));
    expect(eq(rejected_naming_a(read<c>(43u)), true));
  };

  "a read consults gte"_test = [] constexpr {
    constexpr auto c = s2s::gte(42u);
    expect(eq(read<c>(42u).has_value(), true));
    expect(eq(rejected_naming_a(read<c>(41u)), true));
  };

  "a read consults any_of"_test = [] constexpr {
    constexpr auto c = s2s::any_of(21u, 42u, 84u);
    expect(eq(read<c>(21u).has_value(), true));
    expect(eq(read<c>(84u).has_value(), true));
    expect(eq(rejected_naming_a(read<c>(43u)), true));
  };

  // The check runs after the field is read, so a constraint cannot rescue a
  // buffer that ran out — the reason has to stay exhaustion.
  "exhaustion outranks a constraint that would also have failed"_test = [] constexpr {
    constexpr auto c = s2s::eq(42u);
    std::array<u8, 2> buffer{0x2a, 0x00};
    memstream<2> stream(buffer);

    auto res = s2s::struct_cast_le<constrained<c>>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::buffer_exhaustion));
    expect(eq(res.error().failed_at, std::string_view{"a"}));
  };

  // Reading stops at the offending field: a constraint failure on the first
  // field must not leave the second one populated from the wire.
  "a violated constraint stops the read at that field"_test = [] constexpr {
    using two =
      s2s::struct_field_list<
        s2s::basic_field<"a", u32, 4_B, s2s::eq(42u)>,
        s2s::basic_field<"b", u32, 4_B>
      >;

    std::array<u8, 8> buffer{0xff, 0x00, 0x00, 0x00, 0x0d, 0xd0, 0xfe, 0xca};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<two>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::validation_failure));
    expect(eq(res.error().failed_at, std::string_view{"a"}));
  };
}
