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
    s2s::magic_number<"magic_num", u32, s2s::field_size<s2s::fixed<4>>, 0xdeadbeef>,
    s2s::magic_byte_array<"magic_arr", 2, std::array<unsigned char, 2>{0xbe, 0xef}>
  >;

constexpr auto populated() -> magic_schema {
  magic_schema obj{};
  obj["magic_str"_f] = s2s::fixed_string<3>("GIF");
  obj["magic_num"_f] = 0xdeadbeef;
  obj["magic_arr"_f] = std::array<unsigned char, 2>{0xbe, 0xef};
  return obj;
}

template <bool big_endian>
constexpr auto write_wrong_magic() -> s2s::cast_result {
  std::array<u8, 10> buffer{};
  memstream<10> stream(buffer);
  auto obj = populated();
  obj["magic_num"_f] = 0xbeefbeef;
  if constexpr(big_endian)
    return s2s::stream_cast_be<magic_schema>(stream, obj);
  else
    return s2s::stream_cast_le<magic_schema>(stream, obj);
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

  // The check is on the value the caller supplied, so it fires before any
  // byte order is applied and must fail identically in both.
  "a wrong magic value is rejected in both byte orders"_test = [] constexpr {
    expect(eq(write_wrong_magic<false>().has_value(), false));
    expect(eq(write_wrong_magic<true>().has_value(), false));
    expect(eq(write_wrong_magic<false>().error().failure_reason,
              s2s::error_reason::validation_failure));
    expect(eq(write_wrong_magic<true>().error().failed_at, std::string_view{"magic_num"}));
  };
}
