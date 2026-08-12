#include <array>
#include <expected>
#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"


using namespace s2s_literals;

using u32 = unsigned int;

using u16 = unsigned short;

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

constexpr auto roundtrip_computed() -> bool {
  std::array<u8, 16> buffer{};
  memstream<16> stream(buffer);
  if(!s2s::stream_cast_le<computed_struct>(stream, make_computed(4)))
    return false;
  stream.rewind();
  auto res = s2s::struct_cast_le<computed_struct>(stream);
  return res && (*res)["cells"_f].size() == 4 && (*res)["cells"_f][0] == 0x1122;
}

// The callable cannot be inverted, so a container that disagrees with its
// sources is rejected rather than reconciled.
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

constexpr auto roundtrip_fanout() -> bool {
  std::array<u8, 16> buffer{};
  memstream<16> stream(buffer);
  if(!s2s::stream_cast_le<fanout_struct>(stream, make_fanout(2)))
    return false;
  stream.rewind();
  auto res = s2s::struct_cast_le<fanout_struct>(stream);
  return res && (*res)["len"_f] == 2 && (*res)["a"_f].size() == 2 &&
         (*res)["b"_f].size() == 2;
}

// One length slot cannot describe two containers of different sizes.
constexpr auto write_contradicting_fanout() -> s2s::cast_result {
  std::array<u8, 16> buffer{};
  memstream<16> stream(buffer);
  return s2s::stream_cast_le<fanout_struct>(stream, make_fanout(1));
}

static_assert(roundtrip_computed(), "computed-length constexpr round-trip failed");
static_assert(!write_disagreeing_computed().has_value());
static_assert(
  write_disagreeing_computed().error().failure_reason ==
    s2s::error_reason::found_contradicting_length);
static_assert(write_disagreeing_computed().error().failed_at == "cells");
static_assert(roundtrip_fanout(), "fan-out constexpr round-trip failed");
static_assert(!write_contradicting_fanout().has_value());
static_assert(write_contradicting_fanout().error().failed_at == "len");
static_assert(
  write_contradicting_fanout().error().failure_reason ==
    s2s::error_reason::found_contradicting_length);

auto main() -> int {
  return 0;
}
