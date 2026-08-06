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

using magic_struct =
  s2s::struct_field_list<
    s2s::magic_string<"magic_str", "GIF">,
    s2s::magic_number<"magic_num", u32, s2s::field_size<s2s::fixed<4>>, 0xdeadbeef>,
    s2s::magic_byte_array<"magic_arr", 2, std::array<unsigned char, 2>{0xbe, 0xef}>
  >;

constexpr auto populated_magic() -> magic_struct {
  magic_struct obj{};
  obj["magic_str"_f] = s2s::fixed_string<3>("GIF");
  obj["magic_num"_f] = 0xdeadbeef;
  obj["magic_arr"_f] = std::array<unsigned char, 2>{0xbe, 0xef};
  return obj;
}

template <bool big_endian>
constexpr auto roundtrip_magic() -> bool {
  std::array<u8, 10> buffer{};
  memstream<10> stream(buffer);
  if constexpr(big_endian) {
    if(!s2s::struct_write_be<magic_struct>(stream, populated_magic()))
      return false;
  } else {
    if(!s2s::struct_write_le<magic_struct>(stream, populated_magic()))
      return false;
  }
  stream.rewind();
  auto res = [&] {
    if constexpr(big_endian)
      return s2s::struct_cast_be<magic_struct>(stream);
    else
      return s2s::struct_cast_le<magic_struct>(stream);
  }();
  return res && (*res)["magic_str"_f] == s2s::fixed_string<3>("GIF") &&
         (*res)["magic_num"_f] == 0xdeadbeef;
}

template <bool big_endian>
constexpr auto write_wrong_magic() -> s2s::cast_result {
  std::array<u8, 10> buffer{};
  memstream<10> stream(buffer);
  auto obj = populated_magic();
  obj["magic_num"_f] = 0xbeefbeef;
  if constexpr(big_endian)
    return s2s::struct_write_be<magic_struct>(stream, obj);
  else
    return s2s::struct_write_le<magic_struct>(stream, obj);
}

using prefixed_struct =
  s2s::struct_field_list<
    s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::vec_field<"vec", u16, s2s::field_size<s2s::len_from_field<"len">>>
  >;

// The vector never escapes the constant evaluation, so its allocation is
// transient and the round-trip is legal at compile time.
template <bool big_endian>
constexpr auto roundtrip_prefixed() -> bool {
  std::array<u8, 10> buffer{};
  memstream<10> stream(buffer);
  prefixed_struct obj{};
  obj["vec"_f] = std::vector<u16>{0x1122, 0x3344, 0x5566};

  if constexpr(big_endian) {
    if(!s2s::struct_write_be<prefixed_struct>(stream, obj))
      return false;
  } else {
    if(!s2s::struct_write_le<prefixed_struct>(stream, obj))
      return false;
  }
  stream.rewind();
  auto res = [&] {
    if constexpr(big_endian)
      return s2s::struct_cast_be<prefixed_struct>(stream);
    else
      return s2s::struct_cast_le<prefixed_struct>(stream);
  }();
  return res && (*res)["len"_f] == 3 && (*res)["vec"_f] == obj["vec"_f];
}

// The length slot is never populated by the caller — since issue 005 it
// cannot be — so what reaches the stream comes from the container alone.
constexpr auto derived_length_bytes() -> bool {
  std::array<u8, 10> buffer{};
  memstream<10> stream(buffer);
  prefixed_struct obj{};
  obj["vec"_f] = std::vector<u16>{0x1122, 0x3344, 0x5566};
  if(!s2s::struct_write_be<prefixed_struct>(stream, obj))
    return false;
  return buffer == std::array<u8, 10>{
    0x00, 0x00, 0x00, 0x03, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
}

using narrow_prefixed_struct =
  s2s::struct_field_list<
    s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<1>>>,
    s2s::vec_field<"vec", u8, s2s::field_size<s2s::len_from_field<"len">>>
  >;

constexpr auto write_overlong_container() -> s2s::cast_result {
  std::array<u8, 400> buffer{};
  memstream<400> stream(buffer);
  narrow_prefixed_struct obj{};
  obj["vec"_f] = std::vector<u8>(300, 0x5a);
  return s2s::struct_write_le<narrow_prefixed_struct>(stream, obj);
}

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
  if(!s2s::struct_write_le<computed_struct>(stream, make_computed(4)))
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
  return s2s::struct_write_le<computed_struct>(stream, make_computed(3));
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
  if(!s2s::struct_write_le<fanout_struct>(stream, make_fanout(2)))
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
  return s2s::struct_write_le<fanout_struct>(stream, make_fanout(1));
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
static_assert(roundtrip_magic<false>(), "little-endian magic round-trip failed");
static_assert(roundtrip_magic<true>(), "big-endian magic round-trip failed");
static_assert(!write_wrong_magic<false>().has_value());
static_assert(!write_wrong_magic<true>().has_value());
static_assert(
  write_wrong_magic<false>().error().failure_reason == s2s::error_reason::validation_failure);
static_assert(write_wrong_magic<true>().error().failed_at == "magic_num");
static_assert(roundtrip_prefixed<false>(), "little-endian prefixed round-trip failed");
static_assert(roundtrip_prefixed<true>(), "big-endian prefixed round-trip failed");
static_assert(derived_length_bytes());
static_assert(!write_overlong_container().has_value());
static_assert(
  write_overlong_container().error().failure_reason == s2s::error_reason::validation_failure);
static_assert(write_overlong_container().error().failed_at == "len");
static_assert(roundtrip_computed(), "computed-length constexpr round-trip failed");
static_assert(!write_disagreeing_computed().has_value());
static_assert(
  write_disagreeing_computed().error().failure_reason == s2s::error_reason::validation_failure);
static_assert(write_disagreeing_computed().error().failed_at == "cells");
static_assert(roundtrip_fanout(), "fan-out constexpr round-trip failed");
static_assert(!write_contradicting_fanout().has_value());
static_assert(write_contradicting_fanout().error().failed_at == "len");

auto main() -> int {
  return 0;
}
