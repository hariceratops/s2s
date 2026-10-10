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

constexpr auto populated() -> two_trivials {
  two_trivials obj{};
  obj["a"_f] = 0xdeadbeef;
  obj["b"_f] = 0xcafed00d;
  return obj;
}

using two_floats =
  s2s::struct_field_list<
    s2s::basic_field<"a", float, 4_B>,
    s2s::fixed_array_field<"b", float, 2>
  >;

constexpr auto populated_floats() -> two_floats {
  two_floats obj{};
  obj["a"_f] = 1.5f;
  obj["b"_f] = std::array<float, 2>{2.5f, -3.25f};
  return obj;
}

// ut's eq on a float demands an epsilon; the round trip here is a pure byte
// move with no arithmetic, so the bit pattern is expected to match exactly,
// and comparing bits sidesteps the epsilon requirement without weakening the
// assertion.
constexpr auto bits_of(float v) -> std::uint32_t {
  return std::bit_cast<std::uint32_t>(v);
}

auto main() -> int {
  "little endian trivials round trip"_test = [] constexpr {
    std::array<u8, 8> buffer{};
    memstream<8> stream(buffer);

    expect(eq(s2s::stream_cast_le<two_trivials>(stream, populated()).has_value(), true));
    stream.rewind();
    auto res = s2s::struct_cast_le<two_trivials>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["a"_f], 0xdeadbeefu));
    expect(eq((*res)["b"_f], 0xcafed00du));
  };

  "big endian trivials round trip"_test = [] constexpr {
    std::array<u8, 8> buffer{};
    memstream<8> stream(buffer);

    expect(eq(s2s::stream_cast_be<two_trivials>(stream, populated()).has_value(), true));
    stream.rewind();
    auto res = s2s::struct_cast_be<two_trivials>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["a"_f], 0xdeadbeefu));
    expect(eq((*res)["b"_f], 0xcafed00du));
  };

  // std::byteswap has no floating-point overload, and a schema containing a
  // float reaches both the host and foreign arms of write_impl once byte
  // order is a runtime value, so both directions have to actually work on one.
  "little endian float fields round trip"_test = [] constexpr {
    std::array<u8, 12> buffer{};
    memstream<12> stream(buffer);

    expect(eq(s2s::stream_cast_le<two_floats>(stream, populated_floats()).has_value(), true));
    stream.rewind();
    auto res = s2s::struct_cast_le<two_floats>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(bits_of((*res)["a"_f]), bits_of(1.5f)));
    expect(eq(bits_of((*res)["b"_f][0]), bits_of(2.5f)));
    expect(eq(bits_of((*res)["b"_f][1]), bits_of(-3.25f)));
  };

  "big endian float fields round trip"_test = [] constexpr {
    std::array<u8, 12> buffer{};
    memstream<12> stream(buffer);

    expect(eq(s2s::stream_cast_be<two_floats>(stream, populated_floats()).has_value(), true));
    stream.rewind();
    auto res = s2s::struct_cast_be<two_floats>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(bits_of((*res)["a"_f]), bits_of(1.5f)));
    expect(eq(bits_of((*res)["b"_f][0]), bits_of(2.5f)));
    expect(eq(bits_of((*res)["b"_f][1]), bits_of(-3.25f)));
  };

  // Round-trip alone would pass even if both directions ignored endianness, so
  // pin the bytes that actually reach the stream.
  "the emitted bytes carry the declared byte order"_test = [] constexpr {
    std::array<u8, 8> le_buffer{};
    memstream<8> le_stream(le_buffer);
    expect(eq(s2s::stream_cast_le<two_trivials>(le_stream, populated()).has_value(), true));
    expect(eq(le_buffer[0], u8{0xef}));
    expect(eq(le_buffer[3], u8{0xde}));

    std::array<u8, 8> be_buffer{};
    memstream<8> be_stream(be_buffer);
    expect(eq(s2s::stream_cast_be<two_trivials>(be_stream, populated()).has_value(), true));
    expect(eq(be_buffer[0], u8{0xde}));
    expect(eq(be_buffer[3], u8{0xef}));
  };

  // One byte short of the schema: the write reports exhaustion rather than
  // silently truncating.
  "an undersized buffer fails on the field that did not fit"_test = [] constexpr {
    std::array<u8, 7> buffer{};
    memstream<7> stream(buffer);

    auto written = s2s::stream_cast_le<two_trivials>(stream, populated());

    expect(eq(written.has_value(), false));
    expect(eq(written.error().failure_reason, s2s::buffer_exhaustion));
    expect(eq(written.error().failed_at, std::string_view{"b"}));
  };
}
