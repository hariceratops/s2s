// The allocation ceiling: declared per field with max_bytes, and applied by
// default where nothing is declared.
//
// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.
//
// This file verifies an *option*, not a container kind, which is why it is its
// own pair rather than additions to variable_buffer_read*.

#include <array>
#include <cstddef>
#include <string_view>
#include <variant>
#include <vector>
#include <ut>

#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

// max_bytes<8> over a u16 element admits four elements and no more.
using bounded_vec =
  s2s::struct_field_list<
    s2s::basic_field<"n", u32, 4_B>,
    s2s::vec_field<"v", u16, s2s::len_from_field<"n">, s2s::max_bytes<8>>
  >;

using bounded_str =
  s2s::struct_field_list<
    s2s::basic_field<"n", u32, 4_B>,
    s2s::str_field<"s", s2s::len_from_field<"n">, s2s::max_bytes<4>>
  >;

// Ten bytes is not a whole number of u32s: the ceiling truncates to two.
using ragged_bound =
  s2s::struct_field_list<
    s2s::basic_field<"n", u32, 4_B>,
    s2s::vec_field<"v", u32, s2s::len_from_field<"n">, s2s::max_bytes<10>>
  >;

using point = s2s::struct_field_list<s2s::basic_field<"x", u16, 2_B>>;

using bounded_records =
  s2s::struct_field_list<
    s2s::basic_field<"n", u32, 4_B>,
    s2s::vector_of_records<"r", point, s2s::len_from_field<"n">, s2s::max_bytes<64>>
  >;

// Nothing declared: the default ceiling is the only thing standing here.
using undeclared =
  s2s::struct_field_list<
    s2s::basic_field<"n", u32, 4_B>,
    s2s::vec_field<"v", u16, s2s::len_from_field<"n">>
  >;

auto main() -> int {
  "a length under its declared bound is read"_test = [] constexpr {
    std::array<u8, 8> buffer{0x02, 0x00, 0x00, 0x00, 0x22, 0x11, 0x44, 0x33};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<bounded_vec>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["v"_f].size(), std::size_t{2}));
  };

  // Inclusive. Getting this backwards is the likeliest silent error in the
  // feature, and it is why the at-bound case is spelled out separately.
  "a length exactly at its declared bound is read"_test = [] constexpr {
    std::array<u8, 12> buffer{0x04, 0x00, 0x00, 0x00,
                              0x22, 0x11, 0x44, 0x33, 0x66, 0x55, 0x88, 0x77};
    memstream<12> stream(buffer);

    auto res = s2s::struct_cast_le<bounded_vec>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["v"_f].size(), std::size_t{4}));
  };

  // One element past the ceiling, and the stream is long enough to satisfy it —
  // so a rejection here is the bound talking, not the buffer running out.
  "a length over its declared bound is rejected"_test = [] constexpr {
    std::array<u8, 14> buffer{0x05, 0x00, 0x00, 0x00,
                              0x22, 0x11, 0x44, 0x33, 0x66, 0x55, 0x88, 0x77, 0x00, 0x00};
    memstream<14> stream(buffer);

    auto res = s2s::struct_cast_le<bounded_vec>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::excessive_length));
    expect(eq(res.error().failed_at, std::string_view{"v"}));
  };

  "a string is bounded by the same ceiling"_test = [] constexpr {
    std::array<u8, 9> buffer{0x05, 0x00, 0x00, 0x00, 'h', 'e', 'l', 'l', 'o'};
    memstream<9> stream(buffer);

    auto res = s2s::struct_cast_le<bounded_str>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::excessive_length));
  };

  // The comparison is a division, so a ceiling that is not a whole number of
  // elements truncates down — 10 bytes admits two u32s, not two and a half.
  "a bound that is not a multiple of the element size truncates down"_test = [] constexpr {
    std::array<u8, 12> buffer{0x02, 0x00, 0x00, 0x00,
                              0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe, 0xca};
    memstream<12> stream(buffer);
    auto two = s2s::struct_cast_le<ragged_bound>(stream);
    expect(eq(two.has_value(), true));

    std::array<u8, 16> wider{0x03, 0x00, 0x00, 0x00,
                             0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe, 0xca,
                             0x00, 0x00, 0x00, 0x00};
    memstream<16> wider_stream(wider);
    auto three = s2s::struct_cast_le<ragged_bound>(wider_stream);
    expect(eq(three.has_value(), false));
    expect(eq(three.error().failure_reason, s2s::error_reason::excessive_length));
  };

  // The denominator is the record's in-memory footprint, which has no
  // relationship to how many bytes those records occupy on the wire.
  "a record vector is bounded by its footprint, not its wire size"_test = [] constexpr {
    std::array<u8, 6> buffer{0x01, 0x00, 0x00, 0x00, 0x22, 0x11};
    memstream<6> stream(buffer);
    auto one = s2s::struct_cast_le<bounded_records>(stream);
    expect(eq(one.has_value(), true));

    std::array<u8, 6> absurd{0xff, 0xff, 0x00, 0x00, 0x00, 0x00};
    memstream<6> absurd_stream(absurd);
    auto many = s2s::struct_cast_le<bounded_records>(absurd_stream);
    expect(eq(many.has_value(), false));
    expect(eq(many.error().failure_reason, s2s::error_reason::excessive_length));
  };

  // The headline property: a schema that declares nothing is still protected.
  // 0x01000000 u16 elements is 32 MiB, over the 16 MiB default.
  "a field declaring no bound still rejects an absurd length"_test = [] constexpr {
    std::array<u8, 4> buffer{0x00, 0x00, 0x00, 0x01};
    memstream<4> stream(buffer);

    auto res = s2s::struct_cast_le<undeclared>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::excessive_length));
  };
}
