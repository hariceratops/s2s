#include <array>
#include <expected>
#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"


using namespace s2s_literals;

using u32 = unsigned int;

using our_struct =
  s2s::struct_field_list<
    s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>
  >;


constexpr auto populated() -> our_struct {
  our_struct obj{};
  obj["a"_f] = 0xdeadbeef;
  obj["b"_f] = 0xcafed00d;
  return obj;
}

constexpr auto roundtrip_le() -> bool {
  std::array<u8, 8> buffer{};
  memstream<8> stream(buffer);
  if(!s2s::struct_write_le<our_struct>(stream, populated()))
    return false;
  stream.rewind();
  auto res = s2s::struct_cast_le<our_struct>(stream);
  return res && (*res)["a"_f] == 0xdeadbeef && (*res)["b"_f] == 0xcafed00d;
}

constexpr auto roundtrip_be() -> bool {
  std::array<u8, 8> buffer{};
  memstream<8> stream(buffer);
  if(!s2s::struct_write_be<our_struct>(stream, populated()))
    return false;
  stream.rewind();
  auto res = s2s::struct_cast_be<our_struct>(stream);
  return res && (*res)["a"_f] == 0xdeadbeef && (*res)["b"_f] == 0xcafed00d;
}

// A buffer one byte short of the schema: the write must report exhaustion
// rather than silently truncating.
constexpr auto write_into_undersized_buffer() -> s2s::cast_result {
  std::array<u8, 7> buffer{};
  memstream<7> stream(buffer);
  return s2s::struct_write_le<our_struct>(stream, populated());
}

using u16 = unsigned short;

using aggregate_struct =
  s2s::struct_field_list<
    s2s::fixed_array_field<"arr", u16, 3>,
    s2s::fixed_string_field<"name", 4>
  >;

constexpr auto populated_aggregate() -> aggregate_struct {
  aggregate_struct obj{};
  obj["arr"_f] = std::array<u16, 3>{0x1122, 0x3344, 0x5566};
  obj["name"_f] = s2s::fixed_string<4>("abcd");
  return obj;
}

template <bool big_endian>
constexpr auto roundtrip_aggregate() -> bool {
  std::array<u8, 11> buffer{};
  memstream<11> stream(buffer);
  const auto original = populated_aggregate();
  if constexpr(big_endian) {
    if(!s2s::struct_write_be<aggregate_struct>(stream, original))
      return false;
  } else {
    if(!s2s::struct_write_le<aggregate_struct>(stream, original))
      return false;
  }
  stream.rewind();
  auto res = [&] {
    if constexpr(big_endian)
      return s2s::struct_cast_be<aggregate_struct>(stream);
    else
      return s2s::struct_cast_le<aggregate_struct>(stream);
  }();
  return res && (*res)["arr"_f] == original["arr"_f] && (*res)["name"_f] == original["name"_f];
}

// The foreign-endian path must swap each element, so the emitted bytes are
// pinned rather than only checked for round-trip symmetry.
constexpr auto foreign_bytes_are_element_swapped() -> bool {
  std::array<u8, 11> buffer{};
  memstream<11> stream(buffer);
  if(!s2s::struct_write_be<aggregate_struct>(stream, populated_aggregate()))
    return false;
  return buffer == std::array<u8, 11>{
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 'a', 'b', 'c', 'd', '\0'};
}

static_assert(roundtrip_le(), "little-endian constexpr round-trip failed");
static_assert(roundtrip_be(), "big-endian constexpr round-trip failed");
static_assert(!write_into_undersized_buffer().has_value());
static_assert(
  write_into_undersized_buffer().error().failure_reason == s2s::error_reason::buffer_exhaustion);
static_assert(write_into_undersized_buffer().error().failed_at == "b");
static_assert(roundtrip_aggregate<false>(), "little-endian aggregate round-trip failed");
static_assert(roundtrip_aggregate<true>(), "big-endian aggregate round-trip failed");
static_assert(foreign_bytes_are_element_swapped());

auto main() -> int {
  return 0;
}
