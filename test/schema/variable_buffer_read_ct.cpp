// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.
//
// See variable_buffer_write_ct.cpp on why these allocating descriptors are
// readable at compile time at all.

#include <array>
#include <cstddef>
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

using prefixed_vec =
  s2s::struct_field_list<
    s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::vec_field<"vec", u16, s2s::field_size<s2s::len_from_field<"len">>>
  >;

using prefixed_str =
  s2s::struct_field_list<
    s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::str_field<"str", s2s::field_size<s2s::len_from_field<"len">>>
  >;

int main() {
  "a vector takes its element count from the sibling length field"_test = [] constexpr {
    std::array<u8, 10> buffer{
      0x03, 0x00, 0x00, 0x00, 0x22, 0x11, 0x44, 0x33, 0x66, 0x55};
    memstream<10> stream(buffer);

    auto res = s2s::struct_cast_le<prefixed_vec>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["len"_f], 3u));
    expect(eq((*res)["vec"_f].size(), std::size_t{3}));
    expect(eq((*res)["vec"_f][0], u16{0x1122}));
    expect(eq((*res)["vec"_f][1], u16{0x3344}));
    expect(eq((*res)["vec"_f][2], u16{0x5566}));
  };

  // The length is a byte count for a string and an element count for a
  // vector; reading three characters from a length of 3 pins that.
  "a string takes its byte count from the sibling length field"_test = [] constexpr {
    std::array<u8, 7> buffer{0x03, 0x00, 0x00, 0x00, 'a', 'b', 'c'};
    memstream<7> stream(buffer);

    auto res = s2s::struct_cast_le<prefixed_str>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["len"_f], 3u));
    expect(eq((*res)["str"_f].size(), std::size_t{3}));
    expect(eq(std::string_view{(*res)["str"_f]}, std::string_view{"abc"}));
  };

  // The length slot and the elements are separate byte-order decisions, and
  // both have to follow the declared order.
  "a foreign endian prefixed vector swaps its length and its elements"_test = [] constexpr {
    std::array<u8, 10> buffer{
      0x00, 0x00, 0x00, 0x03, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    memstream<10> stream(buffer);

    auto res = s2s::struct_cast_be<prefixed_vec>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["len"_f], 3u));
    expect(eq((*res)["vec"_f][0], u16{0x1122}));
    expect(eq((*res)["vec"_f][2], u16{0x5566}));
  };

  // char has no byte order, so only the length slot moves.
  "a foreign endian prefixed string is not reversed"_test = [] constexpr {
    std::array<u8, 7> buffer{0x00, 0x00, 0x00, 0x03, 'a', 'b', 'c'};
    memstream<7> stream(buffer);

    auto res = s2s::struct_cast_be<prefixed_str>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::string_view{(*res)["str"_f]}, std::string_view{"abc"}));
  };

  "a zero length yields an empty container and consumes nothing more"_test = [] constexpr {
    std::array<u8, 4> buffer{0x00, 0x00, 0x00, 0x00};
    memstream<4> stream(buffer);

    auto res = s2s::struct_cast_le<prefixed_vec>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["len"_f], 0u));
    expect(eq((*res)["vec"_f].empty(), true));
  };

  // The length is trusted from the wire, so a buffer shorter than it claims
  // has to fail rather than return a short container.
  "a length longer than the remaining bytes fails on the container"_test = [] constexpr {
    std::array<u8, 8> buffer{0x03, 0x00, 0x00, 0x00, 0x22, 0x11, 0x44, 0x33};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<prefixed_vec>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::buffer_exhaustion));
    expect(eq(res.error().failed_at, std::string_view{"vec"}));
  };
}
