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
    s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>
  >;

constexpr auto populated() -> two_trivials {
  two_trivials obj{};
  obj["a"_f] = 0xdeadbeef;
  obj["b"_f] = 0xcafed00d;
  return obj;
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
