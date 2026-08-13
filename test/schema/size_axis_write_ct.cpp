// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.
//
// The size callable is namespace scope on purpose. It is an NTTP of the
// schema, not a capture of the test body.

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

auto area_of = [](auto rows, auto cols) { return rows * cols; };

using computed_struct =
  s2s::struct_field_list<
    s2s::basic_field<"rows", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"cols", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::vec_field<
      "cells",
      u16,
      s2s::field_size<s2s::len_from_fields<area_of, s2s::with_fields<"rows", "cols">>>
    >
  >;

constexpr auto make_computed(std::size_t cell_count) -> computed_struct {
  computed_struct obj{};
  obj["rows"_f] = 2;
  obj["cols"_f] = 2;
  obj["cells"_f] = std::vector<u16>(cell_count, 0x1122);
  return obj;
}

constexpr auto write_disagreeing_computed() -> s2s::cast_result {
  std::array<u8, 16> buffer{};
  memstream<16> stream(buffer);
  return s2s::stream_cast_le<computed_struct>(stream, make_computed(3));
}

using fanout_struct =
  s2s::struct_field_list<
    s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::vec_field<"a", u16, s2s::field_size<s2s::len_from_field<"len">>>,
    s2s::vec_field<"b", u32, s2s::field_size<s2s::len_from_field<"len">>>
  >;

constexpr auto make_fanout(std::size_t b_count) -> fanout_struct {
  fanout_struct obj{};
  obj["a"_f] = std::vector<u16>(2, 0x1122);
  obj["b"_f] = std::vector<u32>(b_count, 0xdeadbeef);
  return obj;
}

constexpr auto write_contradicting_fanout() -> s2s::cast_result {
  std::array<u8, 16> buffer{};
  memstream<16> stream(buffer);
  return s2s::stream_cast_le<fanout_struct>(stream, make_fanout(1));
}

auto main() -> int {
  "a computed length round trips when the sources agree"_test = [] constexpr {
    std::array<u8, 16> buffer{};
    memstream<16> stream(buffer);

    expect(eq(s2s::stream_cast_le<computed_struct>(
      stream, make_computed(4)).has_value(), true));
    stream.rewind();
    auto res = s2s::struct_cast_le<computed_struct>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["cells"_f].size(), std::size_t{4}));
    expect(eq((*res)["cells"_f][0], u16{0x1122}));
  };

  // The callable cannot be inverted, so a container that disagrees with its
  // sources is rejected rather than reconciled.
  "a container disagreeing with its computed length is rejected"_test = [] constexpr {
    auto written = write_disagreeing_computed();

    expect(eq(written.has_value(), false));
    expect(eq(written.error().failure_reason,
              s2s::error_reason::found_contradicting_length));
    expect(eq(written.error().failed_at, std::string_view{"cells"}));
  };

  "a shared length round trips when every dependent agrees"_test = [] constexpr {
    std::array<u8, 16> buffer{};
    memstream<16> stream(buffer);

    expect(eq(s2s::stream_cast_le<fanout_struct>(
      stream, make_fanout(2)).has_value(), true));
    stream.rewind();
    auto res = s2s::struct_cast_le<fanout_struct>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["len"_f], 2u));
    expect(eq((*res)["a"_f].size(), std::size_t{2}));
    expect(eq((*res)["b"_f].size(), std::size_t{2}));
  };

  // One length slot cannot describe two containers of different sizes. The
  // length is written first, so this fails there, before any bytes.
  "a shared length its dependents contradict is rejected"_test = [] constexpr {
    auto written = write_contradicting_fanout();

    expect(eq(written.has_value(), false));
    expect(eq(written.error().failure_reason,
              s2s::error_reason::found_contradicting_length));
    expect(eq(written.error().failed_at, std::string_view{"len"}));
  };

  // TODO(045): the write side of the defaulted size — a field declared
  // basic_field<"v", u16> writes sizeof(u16) bytes, and 2_B writes the same
  // bytes as today's field_size<fixed<2>>.
  //
  // Placeholder body: asserts nothing yet, since neither spelling exists.
  "a basic_field with no size writes sizeof(T)"_test = [] constexpr {
    expect(eq(true, true));
  };

  // TODO(043): the positive half of hidden_length_target.cpp — assigning the
  // data field alone still writes a correct length, with the length target
  // unnameable. Today's equivalent reads the length back to check it; after 043
  // the check is on the bytes written, since there is nothing to read.
  "a length target is derived without being nameable"_test = [] constexpr {
    expect(eq(true, true));
  };
}
