// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.

#include <array>
#include <bit>
#include <cstdint>
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

using two_floats =
  s2s::struct_field_list<
    s2s::field<"a", float, 4_B, s2s::no_constraint<float>{}>,
    s2s::fixed_array_field<"b", float, 2>
  >;

constexpr auto byte_reversed(float v) -> float {
  return std::bit_cast<float>(std::byteswap(std::bit_cast<std::uint32_t>(v)));
}

constexpr auto float_bytes(float v) -> std::array<u8, 4> {
  return std::bit_cast<std::array<u8, 4>>(v);
}

// ut's eq on a float demands an epsilon; the round trip here is a pure byte
// move with no arithmetic, so the bit pattern is expected to match exactly,
// and comparing bits sidesteps the epsilon requirement without weakening the
// assertion.
constexpr auto bits_of(float v) -> std::uint32_t {
  return std::bit_cast<std::uint32_t>(v);
}

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

  // std::byteswap has no floating-point overload, and a schema containing a
  // float reaches both the host and foreign arms of read_impl once byte order
  // is a runtime value, so both directions have to actually work on one.
  "little endian float fields take their declared byte order"_test = [] constexpr {
    auto a_bytes = float_bytes(1.5f);
    auto b0_bytes = float_bytes(2.5f);
    auto b1_bytes = float_bytes(-3.25f);
    std::array<u8, 12> buffer{
      a_bytes[0], a_bytes[1], a_bytes[2], a_bytes[3],
      b0_bytes[0], b0_bytes[1], b0_bytes[2], b0_bytes[3],
      b1_bytes[0], b1_bytes[1], b1_bytes[2], b1_bytes[3]
    };
    memstream<12> stream(buffer);

    auto res = s2s::struct_cast_le<two_floats>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(bits_of((*res)["a"_f]), bits_of(1.5f)));
    expect(eq(bits_of((*res)["b"_f][0]), bits_of(2.5f)));
    expect(eq(bits_of((*res)["b"_f][1]), bits_of(-3.25f)));
  };

  "big endian float fields take their declared byte order"_test = [] constexpr {
    auto a_bytes = float_bytes(1.5f);
    auto b0_bytes = float_bytes(2.5f);
    auto b1_bytes = float_bytes(-3.25f);
    std::array<u8, 12> buffer{
      a_bytes[0], a_bytes[1], a_bytes[2], a_bytes[3],
      b0_bytes[0], b0_bytes[1], b0_bytes[2], b0_bytes[3],
      b1_bytes[0], b1_bytes[1], b1_bytes[2], b1_bytes[3]
    };
    memstream<12> stream(buffer);

    auto res = s2s::struct_cast_be<two_floats>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(bits_of((*res)["a"_f]), bits_of(byte_reversed(1.5f))));
    expect(eq(bits_of((*res)["b"_f][0]), bits_of(byte_reversed(2.5f))));
    expect(eq(bits_of((*res)["b"_f][1]), bits_of(byte_reversed(-3.25f))));
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
