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

using u16 = unsigned short;

// fixed_string_field<4> is sized fixed<5>, so the aggregate is 6 + 5 bytes.
using aggregate =
  s2s::struct_field_list<
    s2s::fixed_array_field<"arr", u16, 3>,
    s2s::fixed_string_field<"name", 4>
  >;

constexpr auto populated() -> aggregate {
  aggregate obj{};
  obj["arr"_f] = std::array<u16, 3>{0x1122, 0x3344, 0x5566};
  obj["name"_f] = s2s::fixed_string<4>("abcd");
  return obj;
}

auto main() -> int {
  "little endian aggregates round trip"_test = [] constexpr {
    std::array<u8, 11> buffer{};
    memstream<11> stream(buffer);

    expect(eq(s2s::stream_cast_le<aggregate>(stream, populated()).has_value(), true));
    stream.rewind();
    auto res = s2s::struct_cast_le<aggregate>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["arr"_f][0], u16{0x1122}));
    expect(eq((*res)["arr"_f][1], u16{0x3344}));
    expect(eq((*res)["arr"_f][2], u16{0x5566}));
    expect(eq(std::string_view{(*res)["name"_f].data()}, std::string_view{"abcd"}));
  };

  "big endian aggregates round trip"_test = [] constexpr {
    std::array<u8, 11> buffer{};
    memstream<11> stream(buffer);

    expect(eq(s2s::stream_cast_be<aggregate>(stream, populated()).has_value(), true));
    stream.rewind();
    auto res = s2s::struct_cast_be<aggregate>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["arr"_f][0], u16{0x1122}));
    expect(eq((*res)["arr"_f][1], u16{0x3344}));
    expect(eq((*res)["arr"_f][2], u16{0x5566}));
    expect(eq(std::string_view{(*res)["name"_f].data()}, std::string_view{"abcd"}));
  };

  // A whole-buffer reversal would round-trip just as well, so the emitted
  // bytes are pinned rather than only checked for symmetry.
  "the foreign endian path swaps each element"_test = [] constexpr {
    std::array<u8, 11> buffer{};
    memstream<11> stream(buffer);

    expect(eq(s2s::stream_cast_be<aggregate>(stream, populated()).has_value(), true));
    // Element by element: ut prints a failing value, and std::array has no
    // operator<< for it to print.
    constexpr std::array<u8, 11> expected{
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 'a', 'b', 'c', 'd', '\0'};
    for(std::size_t idx = 0; idx < expected.size(); ++idx)
      expect(eq(buffer[idx], expected[idx]));
  };
}
