// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.
//
// str_field and vec_field allocate, which docs/compile-time.md calls
// disqualifying. That is about a parse whose *result* escapes into a namespace
// scope `constexpr auto`. Inside a test body the allocation is transient, so
// the whole round trip is legal during constant evaluation.

#include <array>
#include <cstddef>
#include <string_view>
#include <vector>
#include <ut>

#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

using u16 = unsigned short;
using u32 = unsigned int;

using prefixed =
  s2s::struct_field_list<
    s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::vec_field<"vec", u16, s2s::field_size<s2s::len_from_field<"len">>>
  >;

constexpr auto populated() -> prefixed {
  prefixed obj{};
  obj["vec"_f] = std::vector<u16>{0x1122, 0x3344, 0x5566};
  return obj;
}

// One byte of length slot cannot describe 300 elements.
using narrow_prefixed =
  s2s::struct_field_list<
    s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<1>>>,
    s2s::vec_field<"vec", u8, s2s::field_size<s2s::len_from_field<"len">>>
  >;

constexpr auto write_overlong_container() -> s2s::cast_result {
  std::array<u8, 400> buffer{};
  memstream<400> stream(buffer);
  narrow_prefixed obj{};
  obj["vec"_f] = std::vector<u8>(300, 0x5a);
  return s2s::stream_cast_le<narrow_prefixed>(stream, obj);
}

int main() {
  "little endian length prefixed fields round trip"_test = [] constexpr {
    std::array<u8, 10> buffer{};
    memstream<10> stream(buffer);

    expect(eq(s2s::stream_cast_le<prefixed>(stream, populated()).has_value(), true));
    stream.rewind();
    auto res = s2s::struct_cast_le<prefixed>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["len"_f], 3u));
    expect(eq((*res)["vec"_f].size(), std::size_t{3}));
    expect(eq((*res)["vec"_f][0], u16{0x1122}));
    expect(eq((*res)["vec"_f][2], u16{0x5566}));
  };

  "big endian length prefixed fields round trip"_test = [] constexpr {
    std::array<u8, 10> buffer{};
    memstream<10> stream(buffer);

    expect(eq(s2s::stream_cast_be<prefixed>(stream, populated()).has_value(), true));
    stream.rewind();
    auto res = s2s::struct_cast_be<prefixed>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["len"_f], 3u));
    expect(eq((*res)["vec"_f].size(), std::size_t{3}));
    expect(eq((*res)["vec"_f][0], u16{0x1122}));
    expect(eq((*res)["vec"_f][2], u16{0x5566}));
  };

  // The length slot is never populated by the caller — since issue 005 it
  // cannot be — so what reaches the stream comes from the container alone.
  "the emitted length comes from the container"_test = [] constexpr {
    std::array<u8, 10> buffer{};
    memstream<10> stream(buffer);

    expect(eq(s2s::stream_cast_be<prefixed>(stream, populated()).has_value(), true));
    constexpr std::array<u8, 10> expected{
      0x00, 0x00, 0x00, 0x03, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    for(std::size_t idx = 0; idx < expected.size(); ++idx)
      expect(eq(buffer[idx], expected[idx]));
  };

  // Silent truncation would produce a stream that reads back as a shorter
  // container, so a length that does not fit its slot fails the write.
  "a length too wide for its slot is rejected"_test = [] constexpr {
    auto written = write_overlong_container();

    expect(eq(written.has_value(), false));
    expect(eq(written.error().failure_reason, s2s::error_reason::validation_failure));
    expect(eq(written.error().failed_at, std::string_view{"len"}));
  };
}
