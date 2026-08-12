// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.

#include <array>
#include <cstddef>
#include <variant>
#include <ut>

#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

using u16 = unsigned short;
using u32 = unsigned int;

using alt_1 =
  s2s::struct_field_list<
    s2s::basic_field<"x", u16, s2s::field_size<s2s::fixed<2>>>
  >;
using alt_2 =
  s2s::struct_field_list<
    s2s::basic_field<"y", u16, s2s::field_size<s2s::fixed<2>>>
  >;

using union_struct =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0xcafed00d, s2s::as_struct<alt_1>>,
          s2s::match_case<0xdeadbeef, s2s::as_struct<alt_2>>
        >
      >
    >
  >;

constexpr auto held_alt_2() -> union_struct {
  alt_2 held{};
  held["y"_f] = 0x1122;
  union_struct obj{};
  obj["body"_f] = held;
  return obj;
}

auto main() -> int {
  "little endian unions round trip"_test = [] constexpr {
    std::array<u8, 6> buffer{};
    memstream<6> stream(buffer);

    expect(eq(s2s::stream_cast_le<union_struct>(stream, held_alt_2()).has_value(), true));
    stream.rewind();
    auto res = s2s::struct_cast_le<union_struct>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["tag"_f], 0xdeadbeefu));
    expect(eq(std::get<alt_2>((*res)["body"_f])["y"_f], u16{0x1122}));
  };

  "big endian unions round trip"_test = [] constexpr {
    std::array<u8, 6> buffer{};
    memstream<6> stream(buffer);

    expect(eq(s2s::stream_cast_be<union_struct>(stream, held_alt_2()).has_value(), true));
    stream.rewind();
    auto res = s2s::struct_cast_be<union_struct>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["tag"_f], 0xdeadbeefu));
    expect(eq(std::get<alt_2>((*res)["body"_f])["y"_f], u16{0x1122}));
  };

  // The discriminant is never stored — 0xdeadbeef comes from the match_case
  // belonging to the held alternative.
  "the discriminant is derived from the held alternative"_test = [] constexpr {
    std::array<u8, 6> buffer{};
    memstream<6> stream(buffer);

    expect(eq(s2s::stream_cast_be<union_struct>(stream, held_alt_2()).has_value(), true));
    constexpr std::array<u8, 6> expected{0xde, 0xad, 0xbe, 0xef, 0x11, 0x22};
    for(std::size_t idx = 0; idx < expected.size(); ++idx)
      expect(eq(buffer[idx], expected[idx]));
  };
}
