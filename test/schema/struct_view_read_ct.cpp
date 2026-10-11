// A view must read what struct_cast reads: every case here builds the same
// bytes into both and compares field by field, in both byte orders.
//
// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.

#include <array>
#include <bit>
#include <cstddef>
#include <span>
#include <ut>

#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

using u16 = unsigned short;
using u32 = unsigned int;
using i16 = short;

// u8, u32, u16 puts the wider fields at misaligned offsets; "narrow" is a
// 3-byte integer in a 4-byte type, and the floats are the other trivials.
using record =
  s2s::struct_field_list<
    s2s::basic_field<"a", u8, 1_B>,
    s2s::basic_field<"b", u32, 4_B>,
    s2s::basic_field<"c", u16, 2_B>,
    s2s::basic_field<"narrow", u32, 3_B>,
    s2s::basic_field<"f", float, 4_B>,
    s2s::basic_field<"d", double, 8_B>,
    s2s::basic_field<"s", i16, 2_B>
  >;

constexpr std::size_t record_width = 24;

constexpr auto sample = std::array<u8, record_width>{
  0x7f,
  0x01, 0x02, 0x03, 0x84,
  0x05, 0x86,
  0x07, 0x08, 0x09,
  0x00, 0x00, 0x80, 0x3f,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x3f,
  0xfe, 0xff
};

// Compares the outcome as well as the values, so the same helper serves the
// success and the short-buffer cases.
template <std::endian order, std::size_t parser_size, typename byte>
constexpr auto view_agrees_with_parser(std::array<u8, parser_size> parser_bytes,
                                       std::span<const byte> view_bytes) -> bool {
  memstream<parser_size> stream(parser_bytes);
  auto parsed = [&] {
    if constexpr(order == std::endian::little)
      return s2s::struct_cast_le<record>(stream);
    else
      return s2s::struct_cast_be<record>(stream);
  }();
  auto viewed = [&] {
    if constexpr(order == std::endian::little)
      return s2s::struct_view_le<record>(view_bytes);
    else
      return s2s::struct_view_be<record>(view_bytes);
  }();

  if(parsed.has_value() != viewed.has_value())
    return false;
  if(!parsed)
    return parsed.error().failure_reason == viewed.error().failure_reason &&
           parsed.error().failed_at == viewed.error().failed_at;

  const auto& p = *parsed;
  const auto& v = *viewed;
  return p["a"_f] == v["a"_f] && p["b"_f] == v["b"_f] && p["c"_f] == v["c"_f] &&
         p["narrow"_f] == v["narrow"_f] && p["f"_f] == v["f"_f] &&
         p["d"_f] == v["d"_f] && p["s"_f] == v["s"_f];
}

auto main() -> int {
  "the record's width is a compile-time constant"_test = [] constexpr {
    expect(eq(s2s::wire_width<record>, record_width));
  };

  "a dynamic-extent view matches the parser in both orders"_test = [] constexpr {
    auto bytes = sample;
    std::span<const u8> span{bytes};
    expect(eq(view_agrees_with_parser<std::endian::little>(bytes, span), true));
    expect(eq(view_agrees_with_parser<std::endian::big>(bytes, span), true));
  };

  "a static-extent view matches the parser in both orders"_test = [] constexpr {
    auto bytes = sample;
    std::span<const u8, record_width> span{bytes};
    expect(eq(view_agrees_with_parser<std::endian::little>(bytes, std::span<const u8>{span}), true));
    auto le = s2s::struct_view_le<record>(span);
    auto be = s2s::struct_view_be<record>(span);
    expect(eq(le.has_value(), true));
    expect(eq(be.has_value(), true));
    expect(eq((*le)["b"_f], 0x84030201u));
    expect(eq((*be)["b"_f], 0x01020384u));
    expect(eq((*le)["narrow"_f], 0x090807u));
    expect(eq((*le)["s"_f], i16{-2}));
    expect(eq((*le)["f"_f] == 1.0f, true));
    expect(eq((*le)["d"_f] == 1.0, true));
  };

  "a mutable span of any accepted byte type is viewable"_test = [] constexpr {
    std::array<char, record_width> as_char{};
    std::array<std::byte, record_width> as_byte{};
    for(std::size_t idx = 0; idx < record_width; ++idx) {
      as_char[idx] = static_cast<char>(sample[idx]);
      as_byte[idx] = static_cast<std::byte>(sample[idx]);
    }
    auto from_char = s2s::struct_view_le<record>(std::span<char>{as_char});
    auto from_byte = s2s::struct_view_le<record>(std::span<const std::byte>{as_byte});
    expect(eq(from_char.has_value(), true));
    expect(eq(from_byte.has_value(), true));
    expect(eq((*from_char)["b"_f], 0x84030201u));
    expect(eq((*from_byte)["c"_f], u16{0x8605}));
  };

  "bytes past the record are ignored"_test = [] constexpr {
    std::array<u8, record_width + 5> larger{};
    for(std::size_t idx = 0; idx < record_width; ++idx)
      larger[idx] = sample[idx];
    larger[record_width + 2] = 0xaa;
    auto view = s2s::struct_view_be<record>(std::span<const u8>{larger});
    expect(eq(view.has_value(), true));
    expect(eq((*view)["a"_f], u8{0x7f}));
    expect(eq((*view)["s"_f], i16{-257}));
  };

  "a view sits at an odd offset into a larger buffer"_test = [] constexpr {
    std::array<u8, record_width + 1> shifted{};
    for(std::size_t idx = 0; idx < record_width; ++idx)
      shifted[idx + 1] = sample[idx];
    auto view = s2s::struct_view_le<record>(std::span<const u8>{shifted}.subspan(1));
    expect(eq(view.has_value(), true));
    expect(eq((*view)["b"_f], 0x84030201u));
    expect(eq((*view)["d"_f] == 1.0, true));
  };

  "a dynamic span short of the record fails like the parser"_test = [] constexpr {
    std::array<u8, record_width - 1> bytes{};
    for(std::size_t idx = 0; idx < bytes.size(); ++idx)
      bytes[idx] = sample[idx];
    std::span<const u8> span{bytes};
    expect(eq(view_agrees_with_parser<std::endian::little>(bytes, span), true));
    expect(eq(view_agrees_with_parser<std::endian::big>(bytes, span), true));
    auto view = s2s::struct_view_le<record>(span);
    expect(eq(view.has_value(), false));
    expect(eq(view.error().failure_reason, s2s::error_reason::buffer_exhaustion));
    expect(eq(view.error().failed_at == "s", true));
  };

  "a span holding only the first field names the next one"_test = [] constexpr {
    std::array<u8, 1> one{0x7f};
    std::span<const u8> span{one};
    expect(eq(view_agrees_with_parser<std::endian::big>(one, span), true));
    auto view = s2s::struct_view_be<record>(span);
    expect(eq(view.has_value(), false));
    expect(eq(view.error().failed_at == "b", true));
  };

  "a read leaves unrelated bytes unread"_test = [] constexpr {
    auto original = sample;
    auto mutated = sample;
    mutated[5] = 0x55;
    mutated[14] = 0x66;
    auto before = s2s::struct_view_le<record>(std::span<const u8>{original});
    auto after = s2s::struct_view_le<record>(std::span<const u8>{mutated});
    expect(eq(before.has_value(), true));
    expect(eq(after.has_value(), true));
    expect(eq((*before)["a"_f] == (*after)["a"_f], true));
    expect(eq((*before)["b"_f] == (*after)["b"_f], true));
    expect(eq((*before)["narrow"_f] == (*after)["narrow"_f], true));
    expect(eq((*before)["c"_f] == (*after)["c"_f], false));
  };

  "a view is copyable and cheap"_test = [] constexpr {
    auto bytes = sample;
    auto view = s2s::struct_view_le<record>(std::span<const u8>{bytes});
    auto copy = *view;
    expect(eq(copy["a"_f] == (*view)["a"_f], true));
    expect(eq(sizeof(copy) == sizeof(const u8*), true));
    expect(eq(std::is_trivially_copyable_v<decltype(copy)>, true));
  };
}
