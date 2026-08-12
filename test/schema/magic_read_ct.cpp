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

// A trailing field on each schema: a magic field that rejects must stop the
// read, and the only way to see that it did is a field behind it.
using magic_num_schema =
  s2s::struct_field_list<
    s2s::magic_number<"magic_num", u32, s2s::field_size<s2s::fixed<4>>, 0xdeadbeef>,
    s2s::basic_field<"payload", u32, s2s::field_size<s2s::fixed<4>>>
  >;

using magic_str_schema =
  s2s::struct_field_list<
    s2s::magic_string<"magic_str", "GIF">,
    s2s::basic_field<"payload", u32, s2s::field_size<s2s::fixed<4>>>
  >;

using magic_arr_schema =
  s2s::struct_field_list<
    s2s::magic_byte_array<"magic_arr", 4, std::array<unsigned char, 4>{0xde, 0xad, 0xbe, 0xef}>,
    s2s::basic_field<"payload", u32, s2s::field_size<s2s::fixed<4>>>
  >;

auto main() -> int {
  "a matching magic number is accepted and kept"_test = [] constexpr {
    std::array<u8, 8> buffer{0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe, 0xca};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<magic_num_schema>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["magic_num"_f], 0xdeadbeefu));
    expect(eq((*res)["payload"_f], 0xcafed00du));
  };

  "a non matching magic number names its field"_test = [] constexpr {
    std::array<u8, 8> buffer{0xef, 0xbe, 0xef, 0xbe, 0x0d, 0xd0, 0xfe, 0xca};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<magic_num_schema>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::validation_failure));
    expect(eq(res.error().failed_at, std::string_view{"magic_num"}));
  };

  // The magic value is a byte pattern, not a number, so reading the same
  // bytes big endian must still match rather than compare a swapped value.
  "a magic number is matched in whichever byte order is declared"_test = [] constexpr {
    std::array<u8, 8> buffer{0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xd0, 0x0d};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_be<magic_num_schema>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["magic_num"_f], 0xdeadbeefu));
    expect(eq((*res)["payload"_f], 0xcafed00du));
  };

  // magic_string<N> is sized fixed<N + 1>, so the terminator is on the wire.
  "a matching magic string is accepted and kept"_test = [] constexpr {
    std::array<u8, 8> buffer{'G', 'I', 'F', '\0', 0x0d, 0xd0, 0xfe, 0xca};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<magic_str_schema>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::string_view{(*res)["magic_str"_f].data()}, std::string_view{"GIF"}));
    expect(eq((*res)["payload"_f], 0xcafed00du));
  };

  "a non matching magic string names its field"_test = [] constexpr {
    std::array<u8, 8> buffer{'N', 'A', 'H', '\0', 0x0d, 0xd0, 0xfe, 0xca};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<magic_str_schema>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::validation_failure));
    expect(eq(res.error().failed_at, std::string_view{"magic_str"}));
  };

  "a matching magic byte array is accepted and kept"_test = [] constexpr {
    std::array<u8, 8> buffer{0xde, 0xad, 0xbe, 0xef, 0x0d, 0xd0, 0xfe, 0xca};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<magic_arr_schema>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["magic_arr"_f][0], static_cast<unsigned char>(0xde)));
    expect(eq((*res)["magic_arr"_f][3], static_cast<unsigned char>(0xef)));
    expect(eq((*res)["payload"_f], 0xcafed00du));
  };

  // One byte off, at the end, so a comparison that stops early would pass.
  "a non matching magic byte array names its field"_test = [] constexpr {
    std::array<u8, 8> buffer{0xde, 0xad, 0xbe, 0x00, 0x0d, 0xd0, 0xfe, 0xca};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<magic_arr_schema>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::validation_failure));
    expect(eq(res.error().failed_at, std::string_view{"magic_arr"}));
  };
}
