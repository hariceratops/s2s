#include <array>
#include <expected>
#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"


using namespace s2s_literals;

using u32 = unsigned int;

using u16 = unsigned short;

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
    if(!s2s::stream_cast_be<magic_struct>(stream, populated_magic()))
      return false;
  } else {
    if(!s2s::stream_cast_le<magic_struct>(stream, populated_magic()))
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
    return s2s::stream_cast_be<magic_struct>(stream, obj);
  else
    return s2s::stream_cast_le<magic_struct>(stream, obj);
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

auto flag_is_set = [](auto flag) { return flag == 0xdeadbeef; };

using optional_struct =
  s2s::struct_field_list<
    s2s::basic_field<"flag", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::maybe<
      s2s::basic_field<"payload", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::parse_if<flag_is_set, s2s::with_fields<"flag">>
    >
  >;

constexpr auto make_optional(u32 flag, bool engaged) -> optional_struct {
  optional_struct obj{};
  obj["flag"_f] = flag;
  if(engaged)
    obj["payload"_f] = 0xcafed00d;
  return obj;
}

template <bool big_endian>
constexpr auto roundtrip_optional_present() -> bool {
  std::array<u8, 8> buffer{};
  memstream<8> stream(buffer);
  const auto obj = make_optional(0xdeadbeef, true);
  if constexpr(big_endian) {
    if(!s2s::stream_cast_be<optional_struct>(stream, obj))
      return false;
  } else {
    if(!s2s::stream_cast_le<optional_struct>(stream, obj))
      return false;
  }
  stream.rewind();
  auto res = [&] {
    if constexpr(big_endian)
      return s2s::struct_cast_be<optional_struct>(stream);
    else
      return s2s::struct_cast_le<optional_struct>(stream);
  }();
  return res && (*res)["payload"_f].has_value() && *(*res)["payload"_f] == 0xcafed00d;
}

// An absent optional contributes no bytes at all, so the flag is the whole
// stream.
constexpr auto roundtrip_optional_absent() -> bool {
  std::array<u8, 8> buffer{};
  memstream<8> stream(buffer);
  if(!s2s::stream_cast_be<optional_struct>(stream, make_optional(0x11223344, false)))
    return false;
  if(buffer != std::array<u8, 8>{0x11, 0x22, 0x33, 0x44, 0x00, 0x00, 0x00, 0x00})
    return false;
  stream.rewind();
  auto res = s2s::struct_cast_be<optional_struct>(stream);
  return res && !(*res)["payload"_f].has_value();
}

constexpr auto write_present_but_empty() -> s2s::cast_result {
  std::array<u8, 8> buffer{};
  memstream<8> stream(buffer);
  return s2s::stream_cast_le<optional_struct>(stream, make_optional(0xdeadbeef, false));
}

constexpr auto write_absent_but_engaged() -> s2s::cast_result {
  std::array<u8, 8> buffer{};
  memstream<8> stream(buffer);
  return s2s::stream_cast_le<optional_struct>(stream, make_optional(0x11223344, true));
}

using conditional_len_struct =
  s2s::struct_field_list<
    s2s::basic_field<"flag", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::maybe<
      s2s::vec_field<"data", u8, s2s::field_size<s2s::len_from_field<"len">>>,
      s2s::parse_if<flag_is_set, s2s::with_fields<"flag">>
    >
  >;

constexpr auto make_conditional(u32 stored_len, bool engaged) -> conditional_len_struct {
  conditional_len_struct obj{};
  obj["flag"_f] = engaged ? 0xdeadbeef : 0x11223344;
  obj["len"_f] = stored_len;
  if(engaged)
    obj["data"_f] = std::vector<u8>{0xaa, 0xbb, 0xcc};
  return obj;
}

// A conditional source is not derived, so the caller's length is used — and
// checked against the container it claims to size.
constexpr auto roundtrip_conditional_len() -> bool {
  std::array<u8, 11> buffer{};
  memstream<11> stream(buffer);
  if(!s2s::stream_cast_be<conditional_len_struct>(stream, make_conditional(3, true)))
    return false;
  stream.rewind();
  auto res = s2s::struct_cast_be<conditional_len_struct>(stream);
  return res && (*res)["len"_f] == 3 && (*res)["data"_f].has_value() &&
         (*res)["data"_f]->size() == 3;
}

constexpr auto write_disagreeing_conditional_len() -> s2s::cast_result {
  std::array<u8, 11> buffer{};
  memstream<11> stream(buffer);
  return s2s::stream_cast_le<conditional_len_struct>(stream, make_conditional(7, true));
}

// Absent producer, no obligation: the stored length is written untouched.
constexpr auto write_unverified_conditional_len() -> s2s::cast_result {
  std::array<u8, 11> buffer{};
  memstream<11> stream(buffer);
  return s2s::stream_cast_le<conditional_len_struct>(stream, make_conditional(7, false));
}

using alt_1 =
  s2s::struct_field_list<
    s2s::basic_field<"x", u16, s2s::field_size<s2s::fixed<2>>>
  >;
using alt_2 =
  s2s::struct_field_list<
    s2s::basic_field<"y", u16, s2s::field_size<s2s::fixed<2>>>
  >;

using union_struct =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0xcafed00d, s2s::as_struct<alt_1>>,
          s2s::match_case<0xdeadbeef, s2s::as_struct<alt_2>>
        >
      >
    >
  >;

constexpr auto held_alt_2() -> union_struct {
  alt_2 held{};
  held["y"_f] = 0x1122;
  union_struct obj{};
  obj["body"_f] = held;
  return obj;
}

template <bool big_endian>
constexpr auto roundtrip_union() -> bool {
  std::array<u8, 6> buffer{};
  memstream<6> stream(buffer);
  if constexpr(big_endian) {
    if(!s2s::stream_cast_be<union_struct>(stream, held_alt_2()))
      return false;
  } else {
    if(!s2s::stream_cast_le<union_struct>(stream, held_alt_2()))
      return false;
  }
  stream.rewind();
  auto res = [&] {
    if constexpr(big_endian)
      return s2s::struct_cast_be<union_struct>(stream);
    else
      return s2s::struct_cast_le<union_struct>(stream);
  }();
  return res && (*res)["tag"_f] == 0xdeadbeef &&
         std::get<alt_2>((*res)["body"_f])["y"_f] == 0x1122;
}

// The discriminant is never stored — 0xdeadbeef comes from the match_case
// belonging to the held alternative.
constexpr auto discriminant_is_derived() -> bool {
  std::array<u8, 6> buffer{};
  memstream<6> stream(buffer);
  if(!s2s::stream_cast_be<union_struct>(stream, held_alt_2()))
    return false;
  return buffer == std::array<u8, 6>{0xde, 0xad, 0xbe, 0xef, 0x11, 0x22};
}

static_assert(roundtrip_magic<false>(), "little-endian magic round-trip failed");
static_assert(roundtrip_magic<true>(), "big-endian magic round-trip failed");
static_assert(!write_wrong_magic<false>().has_value());
static_assert(!write_wrong_magic<true>().has_value());
static_assert(
  write_wrong_magic<false>().error().failure_reason == s2s::error_reason::validation_failure);
static_assert(write_wrong_magic<true>().error().failed_at == "magic_num");
static_assert(roundtrip_computed(), "computed-length constexpr round-trip failed");
static_assert(!write_disagreeing_computed().has_value());
static_assert(
  write_disagreeing_computed().error().failure_reason ==
    s2s::error_reason::found_contradicting_length);
static_assert(write_disagreeing_computed().error().failed_at == "cells");
static_assert(roundtrip_fanout(), "fan-out constexpr round-trip failed");
static_assert(!write_contradicting_fanout().has_value());
static_assert(write_contradicting_fanout().error().failed_at == "len");
static_assert(roundtrip_optional_present<false>(), "little-endian optional round-trip failed");
static_assert(roundtrip_optional_present<true>(), "big-endian optional round-trip failed");
static_assert(roundtrip_optional_absent());
static_assert(!write_present_but_empty().has_value());
static_assert(write_present_but_empty().error().failed_at == "payload");
static_assert(!write_absent_but_engaged().has_value());
static_assert(write_absent_but_engaged().error().failed_at == "payload");
static_assert(roundtrip_conditional_len(), "conditional-length round-trip failed");
static_assert(!write_disagreeing_conditional_len().has_value());
static_assert(
  write_disagreeing_conditional_len().error().failure_reason ==
    s2s::error_reason::found_contradicting_length);
static_assert(write_disagreeing_conditional_len().error().failed_at == "len");
static_assert(write_unverified_conditional_len().has_value());
static_assert(roundtrip_union<false>(), "little-endian union round-trip failed");
static_assert(roundtrip_union<true>(), "big-endian union round-trip failed");
static_assert(discriminant_is_derived());
static_assert(
  write_contradicting_fanout().error().failure_reason ==
    s2s::error_reason::found_contradicting_length);

auto main() -> int {
  return 0;
}
