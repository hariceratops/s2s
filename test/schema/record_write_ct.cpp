// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.

#include <array>
#include <cstddef>
#include <vector>
#include <ut>

#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

using u16 = unsigned short;

using point =
  s2s::struct_field_list<
    s2s::basic_field<"x", u16, s2s::field_size<s2s::fixed<2>>>,
    s2s::basic_field<"y", u16, s2s::field_size<s2s::fixed<2>>>
  >;

// All three record descriptors in one schema, so their relative offsets are
// pinned along with their contents.
using records =
  s2s::struct_field_list<
    s2s::struct_field<"origin", point>,
    s2s::array_of_records<"corners", point, 2>,
    s2s::basic_field<"count", u16, s2s::field_size<s2s::fixed<2>>>,
    s2s::vector_of_records<"path", point, s2s::field_size<s2s::len_from_field<"count">>>
  >;

constexpr auto make_point(u16 x, u16 y) -> point {
  point p{};
  p["x"_f] = x;
  p["y"_f] = y;
  return p;
}

constexpr auto populated() -> records {
  records obj{};
  obj["origin"_f] = make_point(0x1111, 0x2222);
  obj["corners"_f][0] = make_point(0x3333, 0x4444);
  obj["corners"_f][1] = make_point(0x5555, 0x6666);
  obj["path"_f] = std::vector<point>{make_point(0x7777, 0x8888)};
  return obj;
}

auto main() -> int {
  "little endian records round trip"_test = [] constexpr {
    std::array<u8, 18> buffer{};
    memstream<18> stream(buffer);

    expect(eq(s2s::stream_cast_le<records>(stream, populated()).has_value(), true));
    stream.rewind();
    auto res = s2s::struct_cast_le<records>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["origin"_f]["x"_f], u16{0x1111}));
    expect(eq((*res)["corners"_f][1]["y"_f], u16{0x6666}));
    expect(eq((*res)["path"_f].size(), std::size_t{1}));
    expect(eq((*res)["path"_f][0]["y"_f], u16{0x8888}));
  };

  "big endian records round trip"_test = [] constexpr {
    std::array<u8, 18> buffer{};
    memstream<18> stream(buffer);

    expect(eq(s2s::stream_cast_be<records>(stream, populated()).has_value(), true));
    stream.rewind();
    auto res = s2s::struct_cast_be<records>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["origin"_f]["x"_f], u16{0x1111}));
    expect(eq((*res)["corners"_f][1]["y"_f], u16{0x6666}));
    expect(eq((*res)["path"_f].size(), std::size_t{1}));
    expect(eq((*res)["path"_f][0]["y"_f], u16{0x8888}));
  };

  // Every leaf, at every depth, in declaration order and nothing else.
  "nested bytes come out in declaration order"_test = [] constexpr {
    std::array<u8, 18> buffer{};
    memstream<18> stream(buffer);

    expect(eq(s2s::stream_cast_be<records>(stream, populated()).has_value(), true));
    constexpr std::array<u8, 18> expected{
      0x11, 0x11, 0x22, 0x22,
      0x33, 0x33, 0x44, 0x44,
      0x55, 0x55, 0x66, 0x66,
      0x00, 0x01,
      0x77, 0x77, 0x88, 0x88};
    for(std::size_t idx = 0; idx < expected.size(); ++idx)
      expect(eq(buffer[idx], expected[idx]));
  };
}
