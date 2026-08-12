// Smoke test for the compile-time tier itself: proves that a ut suite, the
// memstream helper and s2s compile and pass under both UT_COMPILE_TIME_ONLY
// and UT_RUN_TIME_ONLY. Per-construct coverage belongs in test/schema/, not
// here.
//
// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Import only what is used.
//
// Test lambdas must not capture: ut skips a capturing lambda at compile time
// silently, so a captured variable would turn a compile-time test into a
// run-time-only one with no diagnostic.

#include <array>
#include <ut>

#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

using u32 = unsigned int;

using two_trivials =
  s2s::struct_field_list<
    s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>
  >;

int main() {
  "a little endian trivial pair round trips"_test = [] constexpr {
    std::array<u8, 8> buffer{0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe, 0xca};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<two_trivials>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["a"_f], 0xdeadbeefu));
    expect(eq((*res)["b"_f], 0xcafed00du));
  };

  "a truncated buffer fails on the field that ran out"_test = [] constexpr {
    std::array<u8, 7> buffer{0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe};
    memstream<7> stream(buffer);

    auto res = s2s::struct_cast_le<two_trivials>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::buffer_exhaustion));
    expect(eq(res.error().failed_at, std::string_view{"b"}));
  };
}
