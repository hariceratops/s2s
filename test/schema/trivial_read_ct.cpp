// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.

#include <array>
#include <string_view>
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
    s2s::basic_field<"a", u32, 4_B>,
    s2s::basic_field<"b", u32, 4_B>
  >;

auto main() -> int {
  "little endian trivials take their declared byte order"_test = [] constexpr {
    std::array<u8, 8> buffer{0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe, 0xca};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<two_trivials>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["a"_f], 0xdeadbeefu));
    expect(eq((*res)["b"_f], 0xcafed00du));
  };

  // The same bytes read big endian must give the byte-reversed values, or the
  // endianness axis is not being applied at all.
  "big endian trivials take their declared byte order"_test = [] constexpr {
    std::array<u8, 8> buffer{0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe, 0xca};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_be<two_trivials>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["a"_f], 0xefbeaddeu));
    expect(eq((*res)["b"_f], 0x0dd0fecau));
  };

  "a truncated buffer fails on the field that ran out"_test = [] constexpr {
    std::array<u8, 7> buffer{0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe};
    memstream<7> stream(buffer);

    auto res = s2s::struct_cast_le<two_trivials>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::buffer_exhaustion));
    expect(eq(res.error().failed_at, std::string_view{"b"}));
  };

  "a violated constraint names the offending field"_test = [] constexpr {
    using constrained =
      s2s::struct_field_list<
        s2s::basic_field<"a", u32, 4_B, s2s::eq(0xdeadbeefu)>,
        s2s::basic_field<"b", u32, 4_B, s2s::eq(0xcafed00du)>
      >;

    std::array<u8, 8> buffer{0xef, 0xbe, 0xad, 0xde, 0xef, 0xbe, 0xad, 0xde};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<constrained>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::validation_failure));
    expect(eq(res.error().failed_at, std::string_view{"b"}));
  };
}
