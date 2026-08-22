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

using magic_schema =
  s2s::struct_field_list<
    s2s::magic_string<"magic_str", "GIF">,
    s2s::magic_number<"magic_num", u32, 4_B, 0xdeadbeef>,
    s2s::magic_byte_array<"magic_arr", 2, std::array<unsigned char, 2>{0xbe, 0xef}>
  >;

// Nothing to populate: a magic field has no setter, and the write path takes
// its value from the constraint. The name is kept so the round-trip cases below
// still read as writing a struct rather than a default-constructed placeholder.
constexpr auto populated() -> magic_schema {
  return magic_schema{};
}

constexpr auto write_untouched() -> std::array<u8, 10> {
  std::array<u8, 10> buffer{};
  memstream<10> stream(buffer);
  auto res = s2s::stream_cast_be<magic_schema>(stream, magic_schema{});
  return res.has_value() ? buffer : std::array<u8, 10>{};
}

auto main() -> int {
  "little endian magic fields round trip"_test = [] constexpr {
    std::array<u8, 10> buffer{};
    memstream<10> stream(buffer);

    expect(eq(s2s::stream_cast_le<magic_schema>(stream, populated()).has_value(), true));
    stream.rewind();
    auto res = s2s::struct_cast_le<magic_schema>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::string_view{(*res)["magic_str"_f].data()}, std::string_view{"GIF"}));
    expect(eq((*res)["magic_num"_f], 0xdeadbeefu));
  };

  "big endian magic fields round trip"_test = [] constexpr {
    std::array<u8, 10> buffer{};
    memstream<10> stream(buffer);

    expect(eq(s2s::stream_cast_be<magic_schema>(stream, populated()).has_value(), true));
    stream.rewind();
    auto res = s2s::struct_cast_be<magic_schema>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::string_view{(*res)["magic_str"_f].data()}, std::string_view{"GIF"}));
    expect(eq((*res)["magic_num"_f], 0xdeadbeefu));
  };

  // A struct nobody assigned to still carries the declared magic onto the
  // wire — the value comes off the constraint, not out of the object.
  "an untouched struct writes the declared magic bytes"_test = [] constexpr {
    // Byte by byte rather than array against array: ut formats a failing
    // operand with operator<<, which std::array does not have.
    constexpr auto bytes = write_untouched();
    expect(eq(bytes[0], u8{'G'}));
    expect(eq(bytes[1], u8{'I'}));
    expect(eq(bytes[2], u8{'F'}));
    expect(eq(bytes[3], u8{'\0'}));
    expect(eq(bytes[4], u8{0xde}));
    expect(eq(bytes[5], u8{0xad}));
    expect(eq(bytes[6], u8{0xbe}));
    expect(eq(bytes[7], u8{0xef}));
    expect(eq(bytes[8], u8{0xbe}));
    expect(eq(bytes[9], u8{0xef}));
  };
}
