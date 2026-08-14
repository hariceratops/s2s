// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.
//
// The size callable is namespace scope on purpose. It is an NTTP of the
// schema, not a capture of the test body.

#include <array>
#include <cstddef>
#include <optional>
#include <string_view>
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

auto area_of = [](auto rows, auto cols) { return rows * cols; };

using computed_struct =
  s2s::struct_field_list<
    s2s::basic_field<"rows", u32, 4_B>,
    s2s::basic_field<"cols", u32, 4_B>,
    s2s::vec_field<
      "cells",
      u16,
      s2s::len_from_fields<area_of, "rows", "cols">
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
    s2s::basic_field<"len", u32, 4_B>,
    s2s::vec_field<"a", u16, s2s::len_from_field<"len">>,
    s2s::vec_field<"b", u32, s2s::len_from_field<"len">>
  >;

constexpr auto make_fanout(std::size_t b_count) -> fanout_struct {
  fanout_struct obj{};
  obj["a"_f] = std::vector<u16>(2, 0x1122);
  obj["b"_f] = std::vector<u32>(b_count, 0xdeadbeef);
  return obj;
}

using defaulted_size_struct = s2s::struct_field_list<s2s::basic_field<"a", u16>>;

using hidden_len_struct =
  s2s::struct_field_list<
    s2s::basic_field<"len", u32, 4_B>,
    s2s::vec_field<"vec", u8, s2s::len_from_field<"len">>
  >;

constexpr auto write_contradicting_fanout() -> s2s::cast_result {
  std::array<u8, 16> buffer{};
  memstream<16> stream(buffer);
  return s2s::stream_cast_le<fanout_struct>(stream, make_fanout(1));
}

// Returns the emitted bytes rather than a cast_result: the length slot is the
// only place the derived value is observable now.
constexpr auto write_hidden_len() -> std::optional<std::array<u8, 7>> {
  hidden_len_struct obj{};
  obj["vec"_f] = std::vector<u8>{0x11, 0x22, 0x33};

  std::array<u8, 7> buffer{};
  memstream<7> stream(buffer);
  if(!s2s::stream_cast_le<hidden_len_struct>(stream, obj))
    return std::nullopt;
  return buffer;
}

using odd_width_struct =
  s2s::struct_field_list<
    s2s::basic_field<"v", u32, 3_B>,
    s2s::basic_field<"tail", u8>
  >;

constexpr auto write_odd_width() -> std::optional<std::array<u8, 4>> {
  odd_width_struct obj{};
  obj["v"_f] = 0x112233u;
  obj["tail"_f] = u8{0x7f};

  std::array<u8, 4> buffer{};
  memstream<4> stream(buffer);
  if(!s2s::stream_cast_le<odd_width_struct>(stream, obj))
    return std::nullopt;
  return buffer;
}

constexpr auto write_defaulted_size() -> std::optional<std::array<u8, 2>> {
  defaulted_size_struct obj{};
  obj["a"_f] = u16{0x1122};

  std::array<u8, 2> buffer{};
  memstream<2> stream(buffer);
  if(!s2s::stream_cast_le<defaulted_size_struct>(stream, obj))
    return std::nullopt;
  return buffer;
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

  // The write side of an odd width: three bytes for the u32 and the fourth
  // belonging to "tail", so nothing was rounded up on the way out either.
  "an odd byte count writes exactly what it declares"_test = [] constexpr {
    auto written = write_odd_width();

    expect(eq(written.has_value(), true));
    expect(eq(written->at(0), u8{0x33}));
    expect(eq(written->at(1), u8{0x22}));
    expect(eq(written->at(2), u8{0x11}));
    expect(eq(written->at(3), u8{0x7f}));
  };

  // The write side of the omitted size: two bytes out for a u16, in the
  // declared order.
  "a basic_field with no size writes sizeof(T)"_test = [] constexpr {
    auto written = write_defaulted_size();

    expect(eq(written.has_value(), true));
    expect(eq(written->at(0), u8{0x22}));
    expect(eq(written->at(1), u8{0x11}));
  };

  // The positive half of test/must_not_compile/hidden_length_target.cpp.
  // Nothing assigns or reads "len", so the check is on the bytes: the length
  // slot has to carry the container's size even though no caller can name it.
  "a length target is derived without being nameable"_test = [] constexpr {
    auto written = write_hidden_len();

    expect(eq(written.has_value(), true));
    expect(eq(written->at(0), u8{0x03}));
    expect(eq(written->at(4), u8{0x11}));
  };
}
