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
using u32 = unsigned int;

int main() {
  // One schema per descriptor family would leave the four untested together;
  // reading them in one pass also pins their relative offsets.
  "all four const sized descriptors read back"_test = [] constexpr {
    using all_four =
      s2s::struct_field_list<
        s2s::fixed_array_field<"arr", u16, 3>,
        s2s::c_arr_field<"carr", u16, 2>,
        s2s::fixed_string_field<"name", 4>,
        s2s::c_str_field<"cstr", 3>
      >;

    std::array<u8, 19> buffer{
      0x22, 0x11, 0x44, 0x33, 0x66, 0x55,
      0xbb, 0xaa, 0xdd, 0xcc,
      'a', 'b', 'c', 'd', '\0',
      'x', 'y', 'z', '\0'
    };
    memstream<19> stream(buffer);

    auto res = s2s::struct_cast_le<all_four>(stream);

    expect(eq(res.has_value(), true));
    auto fields = *res;
    // Compared element by element throughout: ut prints a failing value, and
    // std::array has no operator<< for it to print.
    expect(eq(fields["arr"_f][0], u16{0x1122}));
    expect(eq(fields["arr"_f][1], u16{0x3344}));
    expect(eq(fields["arr"_f][2], u16{0x5566}));
    expect(eq(fields["carr"_f][0], u16{0xaabb}));
    expect(eq(fields["carr"_f][1], u16{0xccdd}));
    expect(eq(std::string_view{fields["name"_f].data()}, std::string_view{"abcd"}));
    expect(eq(std::string_view{fields["cstr"_f]}, std::string_view{"xyz"}));
  };

  // A whole-buffer reversal would give 0x6655, 0x4433, 0x2211 and pass any
  // round-trip check, so the values are pinned against known foreign bytes.
  "a foreign endian buffer is swapped per element"_test = [] constexpr {
    using schema =
      s2s::struct_field_list<
        s2s::fixed_array_field<"arr", u16, 3>,
        s2s::fixed_string_field<"name", 4>
      >;

    std::array<u8, 11> buffer{
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 'a', 'b', 'c', 'd', '\0'};
    memstream<11> stream(buffer);

    auto res = s2s::struct_cast_be<schema>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["arr"_f][0], u16{0x1122}));
    expect(eq((*res)["arr"_f][1], u16{0x3344}));
    expect(eq((*res)["arr"_f][2], u16{0x5566}));
    // char has no byte order: the string must not come back reversed.
    expect(eq(std::string_view{(*res)["name"_f].data()}, std::string_view{"abcd"}));
  };

  // The swap has to descend into the inner array rather than treating each
  // row as one opaque element.
  "the swap descends into a nested aggregate"_test = [] constexpr {
    using schema =
      s2s::struct_field_list<s2s::fixed_array_field<"arr", std::array<u16, 2>, 4>>;

    std::array<u8, 16> buffer{
      0x11, 0x22, 0x33, 0x44,
      0x55, 0x66, 0x77, 0x88,
      0x99, 0xaa, 0xbb, 0xcc,
      0xdd, 0xee, 0xff, 0x00
    };
    memstream<16> stream(buffer);

    auto res = s2s::struct_cast_be<schema>(stream);

    expect(eq(res.has_value(), true));
    constexpr std::array<std::array<u16, 2>, 4> expected{{
      {0x1122, 0x3344}, {0x5566, 0x7788}, {0x99aa, 0xbbcc}, {0xddee, 0xff00}}};
    for(std::size_t row = 0; row < expected.size(); ++row)
      for(std::size_t col = 0; col < expected[row].size(); ++col)
        expect(eq((*res)["arr"_f][row][col], expected[row][col]));
  };

  // Both string descriptors are sized N + 1. A trailing field is the only way
  // to observe that the terminator was consumed rather than left on the wire.
  "a fixed string consumes its terminator"_test = [] constexpr {
    using schema =
      s2s::struct_field_list<
        s2s::fixed_string_field<"name", 4>,
        s2s::basic_field<"tail", u16, s2s::field_size<s2s::fixed<2>>>
      >;

    std::array<u8, 7> buffer{'a', 'b', 'c', 'd', '\0', 0x22, 0x11};
    memstream<7> stream(buffer);

    auto res = s2s::struct_cast_le<schema>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::string_view{(*res)["name"_f].data()}, std::string_view{"abcd"}));
    expect(eq((*res)["tail"_f], u16{0x1122}));
  };

  "a c string consumes its terminator"_test = [] constexpr {
    using schema =
      s2s::struct_field_list<
        s2s::c_str_field<"cstr", 3>,
        s2s::basic_field<"tail", u16, s2s::field_size<s2s::fixed<2>>>
      >;

    std::array<u8, 6> buffer{'x', 'y', 'z', '\0', 0x22, 0x11};
    memstream<6> stream(buffer);

    auto res = s2s::struct_cast_le<schema>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::string_view{(*res)["cstr"_f]}, std::string_view{"xyz"}));
    expect(eq((*res)["tail"_f], u16{0x1122}));
  };

  // One byte short of the terminator, so the string itself is what runs out.
  "a buffer too short for the terminator fails on the string"_test = [] constexpr {
    using schema = s2s::struct_field_list<s2s::fixed_string_field<"name", 4>>;

    std::array<u8, 4> buffer{'a', 'b', 'c', 'd'};
    memstream<4> stream(buffer);

    auto res = s2s::struct_cast_le<schema>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::buffer_exhaustion));
    expect(eq(res.error().failed_at, std::string_view{"name"}));
  };

  "a c array field reads its declared byte order"_test = [] constexpr {
    using schema = s2s::struct_field_list<s2s::c_arr_field<"carr", u32, 2>>;

    std::array<u8, 8> buffer{0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe, 0xca};
    memstream<8> stream(buffer);

    auto le = s2s::struct_cast_le<schema>(stream);
    expect(eq(le.has_value(), true));
    expect(eq((*le)["carr"_f][0], 0xdeadbeefu));
    expect(eq((*le)["carr"_f][1], 0xcafed00du));

    stream.rewind();
    auto be = s2s::struct_cast_be<schema>(stream);
    expect(eq(be.has_value(), true));
    expect(eq((*be)["carr"_f][0], 0xefbeaddeu));
    expect(eq((*be)["carr"_f][1], 0x0dd0fecau));
  };
}
