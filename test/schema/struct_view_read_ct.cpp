// A view must read what struct_cast reads: every case here builds the same
// bytes into both and compares field by field, in both byte orders.
//
// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.

#include <array>
#include <bit>
#include <cstddef>
#include <span>
#include <ut>

#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

using u16 = unsigned short;
using u32 = unsigned int;
using i16 = short;

// u8, u32, u16 puts the wider fields at misaligned offsets; "narrow" is a
// 3-byte integer in a 4-byte type, and the floats are the other trivials.
using record =
  s2s::struct_field_list<
    s2s::basic_field<"a", u8, 1_B>,
    s2s::basic_field<"b", u32, 4_B>,
    s2s::basic_field<"c", u16, 2_B>,
    s2s::basic_field<"narrow", u32, 3_B>,
    s2s::basic_field<"f", float, 4_B>,
    s2s::basic_field<"d", double, 8_B>,
    s2s::basic_field<"s", i16, 2_B>
  >;

constexpr std::size_t record_width = 24;

constexpr auto sample = std::array<u8, record_width>{
  0x7f,
  0x01, 0x02, 0x03, 0x84,
  0x05, 0x86,
  0x07, 0x08, 0x09,
  0x00, 0x00, 0x80, 0x3f,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x3f,
  0xfe, 0xff
};

// Compares the outcome as well as the values, so the same helper serves the
// success and the short-buffer cases.
template <std::endian order, std::size_t parser_size, typename byte>
constexpr auto view_agrees_with_parser(std::array<u8, parser_size> parser_bytes,
                                       std::span<const byte> view_bytes) -> bool {
  memstream<parser_size> stream(parser_bytes);
  auto parsed = [&] {
    if constexpr(order == std::endian::little)
      return s2s::struct_cast_le<record>(stream);
    else
      return s2s::struct_cast_be<record>(stream);
  }();
  auto viewed = [&] {
    if constexpr(order == std::endian::little)
      return s2s::struct_view_le<record>(view_bytes);
    else
      return s2s::struct_view_be<record>(view_bytes);
  }();

  if(parsed.has_value() != viewed.has_value())
    return false;
  if(!parsed)
    return parsed.error().failure_reason == viewed.error().failure_reason &&
           parsed.error().failed_at == viewed.error().failed_at;

  const auto& p = *parsed;
  const auto& v = *viewed;
  return p["a"_f] == v["a"_f] && p["b"_f] == v["b"_f] && p["c"_f] == v["c"_f] &&
         p["narrow"_f] == v["narrow"_f] && p["f"_f] == v["f"_f] &&
         p["d"_f] == v["d"_f] && p["s"_f] == v["s"_f];
}

// Fixed-width aggregates (076). The leading u8 puts every array at a
// misaligned offset.
using aggregates =
  s2s::struct_field_list<
    s2s::basic_field<"a", u8, 1_B>,
    s2s::fixed_array_field<"arr", u16, 3>,
    s2s::c_arr_field<"carr", u32, 2>,
    s2s::c_str_field<"cs", 3>,
    s2s::fixed_string_field<"fs", 4>
  >;

constexpr std::size_t aggregates_width = 24;

constexpr auto aggregates_sample = std::array<u8, aggregates_width>{
  0x7f,
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
  0x11, 0x12, 0x13, 0x14, 0x21, 0x22, 0x23, 0x24,
  'a', 'b', 'c', 0,
  'w', 'x', 'y', 'z', 0
};

template <std::endian order>
constexpr auto aggregates_agree_with_parser(std::span<const u8> view_bytes) -> bool {
  auto bytes = aggregates_sample;
  memstream<aggregates_width> stream(bytes);
  auto parsed = [&] {
    if constexpr(order == std::endian::little)
      return s2s::struct_cast_le<aggregates>(stream);
    else
      return s2s::struct_cast_be<aggregates>(stream);
  }();
  auto viewed = [&] {
    if constexpr(order == std::endian::little)
      return s2s::struct_view_le<aggregates>(view_bytes);
    else
      return s2s::struct_view_be<aggregates>(view_bytes);
  }();
  if(!parsed.has_value() || !viewed.has_value())
    return false;

  const auto& p = *parsed;
  const auto& v = *viewed;
  const auto carr = v["carr"_f];
  const auto cs = v["cs"_f];
  bool same = p["a"_f] == v["a"_f] && p["arr"_f] == v["arr"_f] && p["fs"_f] == v["fs"_f];
  for(std::size_t idx = 0; idx < carr.size(); ++idx)
    same = same && p["carr"_f][idx] == carr[idx];
  for(std::size_t idx = 0; idx < cs.size(); ++idx)
    same = same && p["cs"_f][idx] == cs[idx];
  return same;
}

// Constrained fields (077). The unconstrained fields around the constrained one
// are what the mutation case changes: creation must not decode them.
template <auto constraint>
using guarded =
  s2s::struct_field_list<
    s2s::basic_field<"pre", u16, 2_B>,
    s2s::basic_field<"v", u32, 4_B, constraint>,
    s2s::basic_field<"post", u16, 2_B>
  >;

using magic_record =
  s2s::struct_field_list<
    s2s::basic_field<"lead", u8, 1_B>,
    s2s::magic_number<"num", u32, 4_B, 0xdeadbeef>,
    s2s::magic_string<"str", "GIF">,
    s2s::magic_byte_array<"arr", 2, std::array<unsigned char, 2>{0xca, 0xfe}>,
    s2s::basic_field<"tail", u16, 2_B>
  >;

constexpr std::size_t magic_width = 1 + 4 + 4 + 2 + 2;

constexpr auto magic_sample = std::array<u8, magic_width>{
  0x42,
  0xef, 0xbe, 0xad, 0xde,
  'G', 'I', 'F', 0,
  0xca, 0xfe,
  0x01, 0x02
};

// Creation and the parser must agree on whether and how bytes are rejected.
template <typename schema, std::endian order, std::size_t N>
constexpr auto creation_agrees_with_parser(std::array<u8, N> bytes) -> bool {
  memstream<N> stream(bytes);
  auto parsed = [&] {
    if constexpr(order == std::endian::little)
      return s2s::struct_cast_le<schema>(stream);
    else
      return s2s::struct_cast_be<schema>(stream);
  }();
  auto viewed = [&] {
    if constexpr(order == std::endian::little)
      return s2s::struct_view_le<schema>(std::span<const u8>{bytes});
    else
      return s2s::struct_view_be<schema>(std::span<const u8>{bytes});
  }();
  if(parsed.has_value() != viewed.has_value())
    return false;
  if(parsed)
    return true;
  return parsed.error().failure_reason == viewed.error().failure_reason &&
         parsed.error().failed_at == viewed.error().failed_at;
}

// Rejects exactly when the stated bytes violate the constraint, in both orders.
template <auto constraint>
constexpr auto guard_rejects(u32 first_byte_of_v, bool expected_ok) -> bool {
  std::array<u8, 8> bytes{0x01, 0x02, static_cast<u8>(first_byte_of_v), 0, 0, 0, 0x05, 0x06};
  auto le = s2s::struct_view_le<guarded<constraint>>(std::span<const u8>{bytes});
  auto be_bytes = std::array<u8, 8>{0x01, 0x02, 0, 0, 0, static_cast<u8>(first_byte_of_v), 0x05, 0x06};
  auto be = s2s::struct_view_be<guarded<constraint>>(std::span<const u8>{be_bytes});
  if(le.has_value() != expected_ok || be.has_value() != expected_ok)
    return false;
  if(!expected_ok && (le.error().failed_at != "v" || be.error().failed_at != "v" ||
                      le.error().failure_reason != s2s::error_reason::validation_failure))
    return false;
  return creation_agrees_with_parser<guarded<constraint>, std::endian::little>(bytes) &&
         creation_agrees_with_parser<guarded<constraint>, std::endian::big>(be_bytes);
}

// Nested records (078). The u8 lead puts the header at an odd offset, and the
// header's own u32 at another.
using inner_header =
  s2s::struct_field_list<
    s2s::basic_field<"kind", u8, 1_B>,
    s2s::basic_field<"len", u32, 4_B>,
    s2s::basic_field<"narrow", u32, 3_B>
  >;
using middle =
  s2s::struct_field_list<
    s2s::basic_field<"m", u16, 2_B>,
    s2s::struct_field<"hdr", inner_header>
  >;
using nested_record =
  s2s::struct_field_list<
    s2s::basic_field<"lead", u8, 1_B>,
    s2s::struct_field<"mid", middle>,
    s2s::basic_field<"tail", u16, 2_B>
  >;
constexpr std::size_t nested_width = 1 + (2 + (1 + 4 + 3)) + 2;

constexpr auto nested_sample = std::array<u8, nested_width>{
  0x7f,
  0x0a, 0x0b,
  0x01,
  0x02, 0x03, 0x04, 0x85,
  0x07, 0x08, 0x09,
  0x0c, 0x0d
};

template <std::endian order>
constexpr auto nested_agrees_with_parser() -> bool {
  auto bytes = nested_sample;
  memstream<nested_width> stream(bytes);
  auto parsed = [&] {
    if constexpr(order == std::endian::little)
      return s2s::struct_cast_le<nested_record>(stream);
    else
      return s2s::struct_cast_be<nested_record>(stream);
  }();
  auto viewed = [&] {
    if constexpr(order == std::endian::little)
      return s2s::struct_view_le<nested_record>(std::span<const u8>{nested_sample});
    else
      return s2s::struct_view_be<nested_record>(std::span<const u8>{nested_sample});
  }();
  if(!parsed.has_value() || !viewed.has_value())
    return false;
  const auto& p = *parsed;
  const auto& v = *viewed;
  const auto sub = v["mid"_f]["hdr"_f];
  return p["lead"_f] == v["lead"_f] && p["tail"_f] == v["tail"_f] &&
         p["mid"_f]["m"_f] == v["mid"_f]["m"_f] &&
         p["mid"_f]["hdr"_f]["kind"_f] == sub["kind"_f] &&
         p["mid"_f]["hdr"_f]["len"_f] == sub["len"_f] &&
         p["mid"_f]["hdr"_f]["narrow"_f] == v["mid"_f]["hdr"_f]["narrow"_f];
}

// Constraints at every depth: a field inside the header, and one on the
// header record itself.
using checked_header =
  s2s::struct_field_list<
    s2s::basic_field<"kind", u8, 1_B>,
    s2s::basic_field<"len", u32, 4_B, s2s::gte(7u)>
  >;
constexpr auto kind_is_len = [](const checked_header& h) { return h["kind"_f] == h["len"_f]; };
using field_checked =
  s2s::struct_field_list<
    s2s::basic_field<"pre", u8, 1_B>,
    s2s::struct_field<"hdr", checked_header>,
    s2s::basic_field<"post", u8, 1_B, s2s::eq(u8{1})>
  >;
using record_checked =
  s2s::struct_field_list<
    s2s::basic_field<"pre", u8, 1_B>,
    s2s::struct_field<"hdr", checked_header, kind_is_len>
  >;

constexpr auto in_range = [](const u32& v) { return v >= 10 && v <= 20; };

// Arrays of records (079). The u8 lead puts the table at an odd offset; each
// entry is 7 bytes, so every element is misaligned differently.
using table_entry =
  s2s::struct_field_list<
    s2s::basic_field<"kind", u8, 1_B>,
    s2s::basic_field<"len", u32, 4_B>,
    s2s::basic_field<"flags", u16, 2_B>
  >;
using table_record =
  s2s::struct_field_list<
    s2s::basic_field<"lead", u8, 1_B>,
    s2s::array_of_records<"entries", table_entry, 3>,
    s2s::basic_field<"tail", u16, 2_B>
  >;
constexpr std::size_t table_width = 1 + 3 * 7 + 2;

constexpr auto table_sample = std::array<u8, table_width>{
  0x7f,
  0x01, 0x10, 0x11, 0x12, 0x13, 0x20, 0x21,
  0x02, 0x30, 0x31, 0x32, 0x33, 0x40, 0x41,
  0x03, 0x50, 0x51, 0x52, 0x53, 0x60, 0x61,
  0x0c, 0x0d
};

template <std::endian order>
constexpr auto table_agrees_with_parser() -> bool {
  auto bytes = table_sample;
  memstream<table_width> stream(bytes);
  auto parsed = [&] {
    if constexpr(order == std::endian::little)
      return s2s::struct_cast_le<table_record>(stream);
    else
      return s2s::struct_cast_be<table_record>(stream);
  }();
  auto viewed = [&] {
    if constexpr(order == std::endian::little)
      return s2s::struct_view_le<table_record>(std::span<const u8>{table_sample});
    else
      return s2s::struct_view_be<table_record>(std::span<const u8>{table_sample});
  }();
  if(!parsed.has_value() || !viewed.has_value())
    return false;
  const auto& p = *parsed;
  const auto& v = *viewed;
  bool same = p["lead"_f] == v["lead"_f] && p["tail"_f] == v["tail"_f];
  for(std::size_t idx = 0; idx < 3; ++idx) {
    const auto entry = v["entries"_f][idx];
    same = same && p["entries"_f][idx]["kind"_f] == entry["kind"_f] &&
           p["entries"_f][idx]["len"_f] == entry["len"_f] &&
           p["entries"_f][idx]["flags"_f] == entry["flags"_f];
  }
  return same;
}

// Arrays nested in records nested in arrays, with a constraint at the bottom.
using leaf_entry =
  s2s::struct_field_list<
    s2s::basic_field<"id", u8, 1_B>,
    s2s::basic_field<"size", u16, 2_B, s2s::gte(u16{5})>
  >;
using group =
  s2s::struct_field_list<
    s2s::basic_field<"n", u8, 1_B>,
    s2s::array_of_records<"leaves", leaf_entry, 2>
  >;
using groups_record =
  s2s::struct_field_list<
    s2s::basic_field<"lead", u8, 1_B>,
    s2s::array_of_records<"groups", group, 2>
  >;
constexpr std::size_t groups_width = 1 + 2 * (1 + 2 * 3);

constexpr auto groups_sample = std::array<u8, groups_width>{
  0x7f,
  0x01, 0xa1, 0x05, 0x00, 0xa2, 0x06, 0x00,
  0x02, 0xb1, 0x07, 0x00, 0xb2, 0x08, 0x00
};

constexpr auto entries_sum_is_96 = [](const std::array<table_entry, 3>& entries) {
  return entries[0]["len"_f] + entries[1]["len"_f] + entries[2]["len"_f] == 96u;
};
using array_checked =
  s2s::struct_field_list<
    s2s::basic_field<"lead", u8, 1_B>,
    s2s::array_of_records<"entries", table_entry, 3, entries_sum_is_96>
  >;
constexpr std::size_t array_checked_width = 1 + 3 * 7;

auto main() -> int {
  "the record's width is a compile-time constant"_test = [] constexpr {
    expect(eq(s2s::wire_width<record>, record_width));
  };

  "a dynamic-extent view matches the parser in both orders"_test = [] constexpr {
    auto bytes = sample;
    std::span<const u8> span{bytes};
    expect(eq(view_agrees_with_parser<std::endian::little>(bytes, span), true));
    expect(eq(view_agrees_with_parser<std::endian::big>(bytes, span), true));
  };

  "a static-extent view matches the parser in both orders"_test = [] constexpr {
    auto bytes = sample;
    std::span<const u8, record_width> span{bytes};
    expect(eq(view_agrees_with_parser<std::endian::little>(bytes, std::span<const u8>{span}), true));
    auto le = s2s::struct_view_le<record>(span);
    auto be = s2s::struct_view_be<record>(span);
    expect(eq(le.has_value(), true));
    expect(eq(be.has_value(), true));
    expect(eq((*le)["b"_f], 0x84030201u));
    expect(eq((*be)["b"_f], 0x01020384u));
    expect(eq((*le)["narrow"_f], 0x090807u));
    expect(eq((*le)["s"_f], i16{-2}));
    expect(eq((*le)["f"_f] == 1.0f, true));
    expect(eq((*le)["d"_f] == 1.0, true));
  };

  "a mutable span of any accepted byte type is viewable"_test = [] constexpr {
    std::array<char, record_width> as_char{};
    std::array<std::byte, record_width> as_byte{};
    for(std::size_t idx = 0; idx < record_width; ++idx) {
      as_char[idx] = static_cast<char>(sample[idx]);
      as_byte[idx] = static_cast<std::byte>(sample[idx]);
    }
    auto from_char = s2s::struct_view_le<record>(std::span<char>{as_char});
    auto from_byte = s2s::struct_view_le<record>(std::span<const std::byte>{as_byte});
    expect(eq(from_char.has_value(), true));
    expect(eq(from_byte.has_value(), true));
    expect(eq((*from_char)["b"_f], 0x84030201u));
    expect(eq((*from_byte)["c"_f], u16{0x8605}));
  };

  "bytes past the record are ignored"_test = [] constexpr {
    std::array<u8, record_width + 5> larger{};
    for(std::size_t idx = 0; idx < record_width; ++idx)
      larger[idx] = sample[idx];
    larger[record_width + 2] = 0xaa;
    auto view = s2s::struct_view_be<record>(std::span<const u8>{larger});
    expect(eq(view.has_value(), true));
    expect(eq((*view)["a"_f], u8{0x7f}));
    expect(eq((*view)["s"_f], i16{-257}));
  };

  "a view sits at an odd offset into a larger buffer"_test = [] constexpr {
    std::array<u8, record_width + 1> shifted{};
    for(std::size_t idx = 0; idx < record_width; ++idx)
      shifted[idx + 1] = sample[idx];
    auto view = s2s::struct_view_le<record>(std::span<const u8>{shifted}.subspan(1));
    expect(eq(view.has_value(), true));
    expect(eq((*view)["b"_f], 0x84030201u));
    expect(eq((*view)["d"_f] == 1.0, true));
  };

  "a dynamic span short of the record fails like the parser"_test = [] constexpr {
    std::array<u8, record_width - 1> bytes{};
    for(std::size_t idx = 0; idx < bytes.size(); ++idx)
      bytes[idx] = sample[idx];
    std::span<const u8> span{bytes};
    expect(eq(view_agrees_with_parser<std::endian::little>(bytes, span), true));
    expect(eq(view_agrees_with_parser<std::endian::big>(bytes, span), true));
    auto view = s2s::struct_view_le<record>(span);
    expect(eq(view.has_value(), false));
    expect(eq(view.error().failure_reason, s2s::error_reason::buffer_exhaustion));
    expect(eq(view.error().failed_at == "s", true));
  };

  "a span holding only the first field names the next one"_test = [] constexpr {
    std::array<u8, 1> one{0x7f};
    std::span<const u8> span{one};
    expect(eq(view_agrees_with_parser<std::endian::big>(one, span), true));
    auto view = s2s::struct_view_be<record>(span);
    expect(eq(view.has_value(), false));
    expect(eq(view.error().failed_at == "b", true));
  };

  "a read leaves unrelated bytes unread"_test = [] constexpr {
    auto original = sample;
    auto mutated = sample;
    mutated[5] = 0x55;
    mutated[14] = 0x66;
    auto before = s2s::struct_view_le<record>(std::span<const u8>{original});
    auto after = s2s::struct_view_le<record>(std::span<const u8>{mutated});
    expect(eq(before.has_value(), true));
    expect(eq(after.has_value(), true));
    expect(eq((*before)["a"_f] == (*after)["a"_f], true));
    expect(eq((*before)["b"_f] == (*after)["b"_f], true));
    expect(eq((*before)["narrow"_f] == (*after)["narrow"_f], true));
    expect(eq((*before)["c"_f] == (*after)["c"_f], false));
  };

  "a view is copyable and cheap"_test = [] constexpr {
    auto bytes = sample;
    auto view = s2s::struct_view_le<record>(std::span<const u8>{bytes});
    auto copy = *view;
    expect(eq(copy["a"_f] == (*view)["a"_f], true));
    expect(eq(sizeof(copy) == sizeof(const u8*), true));
    expect(eq(std::is_trivially_copyable_v<decltype(copy)>, true));
  };

  "fixed arrays and strings match the parser in both orders"_test = [] constexpr {
    expect(eq(aggregates_agree_with_parser<std::endian::little>(aggregates_sample), true));
    expect(eq(aggregates_agree_with_parser<std::endian::big>(aggregates_sample), true));
  };

  "arrays return their whole value and swap per element in foreign order"_test = [] constexpr {
    auto le = s2s::struct_view_le<aggregates>(std::span<const u8, aggregates_width>{aggregates_sample});
    auto be = s2s::struct_view_be<aggregates>(std::span<const u8, aggregates_width>{aggregates_sample});
    expect(eq(le.has_value(), true));
    expect(eq(be.has_value(), true));
    expect(eq((*le)["arr"_f] == std::array<u16, 3>{0x0201, 0x0403, 0x0605}, true));
    expect(eq((*be)["arr"_f] == std::array<u16, 3>{0x0102, 0x0304, 0x0506}, true));
    expect(eq((*le)["carr"_f] == std::array<u32, 2>{0x14131211, 0x24232221}, true));
    expect(eq((*be)["carr"_f] == std::array<u32, 2>{0x11121314, 0x21222324}, true));
  };

  "strings are not swapped and a c string keeps its terminator slot"_test = [] constexpr {
    auto le = s2s::struct_view_le<aggregates>(std::span<const u8>{aggregates_sample});
    auto be = s2s::struct_view_be<aggregates>(std::span<const u8>{aggregates_sample});
    expect(eq((*le)["fs"_f].to_sv() == "wxyz", true));
    expect(eq((*be)["fs"_f].to_sv() == "wxyz", true));
    expect(eq((*be)["cs"_f] == std::array<char, 4>{'a', 'b', 'c', 0}, true));
    expect(eq(std::is_same_v<std::remove_cvref_t<decltype((*le)["cs"_f])>, std::array<char, 4>>, true));
    expect(eq(std::is_same_v<std::remove_cvref_t<decltype((*le)["carr"_f])>, std::array<u32, 2>>, true));
    expect(eq(std::is_const_v<decltype((*le)["arr"_f])>, true));
  };

  "a dynamic span short of the aggregates names the first field that does not fit"_test = [] constexpr {
    auto view = s2s::struct_view_le<aggregates>(std::span<const u8>{aggregates_sample}.first(aggregates_width - 1));
    expect(eq(view.has_value(), false));
    expect(eq(view.error().failed_at == "fs", true));
  };

  "creation rejects a violation of each constraint kind like the parser"_test = [] constexpr {
    expect(eq(guard_rejects<s2s::eq(7u)>(7, true), true));
    expect(eq(guard_rejects<s2s::eq(7u)>(8, false), true));
    expect(eq(guard_rejects<s2s::neq(7u)>(8, true), true));
    expect(eq(guard_rejects<s2s::neq(7u)>(7, false), true));
    expect(eq(guard_rejects<s2s::lt(7u)>(6, true), true));
    expect(eq(guard_rejects<s2s::lt(7u)>(7, false), true));
    expect(eq(guard_rejects<s2s::gt(7u)>(8, true), true));
    expect(eq(guard_rejects<s2s::gt(7u)>(7, false), true));
    expect(eq(guard_rejects<s2s::lte(7u)>(7, true), true));
    expect(eq(guard_rejects<s2s::lte(7u)>(8, false), true));
    expect(eq(guard_rejects<s2s::gte(7u)>(7, true), true));
    expect(eq(guard_rejects<s2s::gte(7u)>(6, false), true));
    expect(eq(guard_rejects<s2s::any_of(1u, 7u, 9u)>(9, true), true));
    expect(eq(guard_rejects<s2s::any_of(1u, 7u, 9u)>(8, false), true));
    expect(eq(guard_rejects<in_range>(15, true), true));
    expect(eq(guard_rejects<in_range>(21, false), true));
  };

  "magic fields are checked at creation and read back like the parser"_test = [] constexpr {
    auto bytes = magic_sample;
    auto le = s2s::struct_view_le<magic_record>(std::span<const u8>{bytes});
    auto be_bytes = bytes;
    be_bytes[1] = 0xde; be_bytes[2] = 0xad; be_bytes[3] = 0xbe; be_bytes[4] = 0xef;
    auto be = s2s::struct_view_be<magic_record>(std::span<const u8>{be_bytes});
    expect(eq(le.has_value(), true));
    expect(eq(be.has_value(), true));
    expect(eq((*le)["num"_f], 0xdeadbeefu));
    expect(eq((*be)["num"_f], 0xdeadbeefu));
    expect(eq(s2s::struct_view_be<magic_record>(std::span<const u8>{bytes}).has_value(), false));
    expect(eq(creation_agrees_with_parser<magic_record, std::endian::big>(bytes), true));
    expect(eq(creation_agrees_with_parser<magic_record, std::endian::big>(be_bytes), true));
    expect(eq((*le)["str"_f].to_sv() == "GIF", true));
    expect(eq((*le)["arr"_f] == std::array<unsigned char, 2>{0xca, 0xfe}, true));
    expect(eq(std::is_same_v<std::remove_cvref_t<decltype((*le)["str"_f])>, s2s::fixed_string<3>>, true));
    expect(eq(creation_agrees_with_parser<magic_record, std::endian::little>(bytes), true));
  };

  "a violated magic field names itself, and the first violation wins"_test = [] constexpr {
    auto bad_num = magic_sample;
    bad_num[1] = 0;
    auto bad_str = magic_sample;
    bad_str[6] = 'X';
    auto bad_arr = magic_sample;
    bad_arr[10] = 0;
    auto both = bad_str;
    both[1] = 0;
    expect(eq(creation_agrees_with_parser<magic_record, std::endian::little>(bad_num), true));
    expect(eq(creation_agrees_with_parser<magic_record, std::endian::big>(bad_num), true));
    expect(eq(creation_agrees_with_parser<magic_record, std::endian::little>(bad_str), true));
    expect(eq(creation_agrees_with_parser<magic_record, std::endian::little>(bad_arr), true));
    expect(eq(creation_agrees_with_parser<magic_record, std::endian::big>(bad_arr), true));
    expect(eq(creation_agrees_with_parser<magic_record, std::endian::little>(both), true));
    expect(eq(s2s::struct_view_le<magic_record>(std::span<const u8>{bad_str}).error().failed_at == "str", true));
    expect(eq(s2s::struct_view_le<magic_record>(std::span<const u8>{both}).error().failed_at == "num", true));
  };

  "a span one byte short fails with exhaustion before any constraint"_test = [] constexpr {
    auto bad = magic_sample;
    bad[1] = 0;
    auto view = s2s::struct_view_le<magic_record>(std::span<const u8>{bad}.first(magic_width - 1));
    expect(eq(view.has_value(), false));
    expect(eq(view.error().failure_reason, s2s::error_reason::buffer_exhaustion));
    expect(eq(view.error().failed_at == "tail", true));
  };

  "changing an unconstrained unread field changes neither creation nor reads"_test = [] constexpr {
    auto original = magic_sample;
    auto mutated = magic_sample;
    mutated[0] = 0x99;
    mutated[11] = 0x77;
    auto before = s2s::struct_view_le<magic_record>(std::span<const u8>{original});
    auto after = s2s::struct_view_le<magic_record>(std::span<const u8>{mutated});
    expect(eq(before.has_value() == after.has_value(), true));
    expect(eq((*before)["num"_f] == (*after)["num"_f], true));
    expect(eq((*before)["arr"_f] == (*after)["arr"_f], true));
    expect(eq((*before)["lead"_f] == (*after)["lead"_f], false));
  };

  "a constrained field is decoded and an unconstrained one is not"_test = [] constexpr {
    expect(eq(s2s::any_field_constrained_v<record>, false));
    expect(eq(s2s::any_field_constrained_v<magic_record>, true));
    expect(eq(s2s::any_field_constrained_v<guarded<s2s::eq(7u)>>, true));
  };

  "a nested record reads as a sub-view that matches the parser in both orders"_test = [] constexpr {
    expect(eq(nested_agrees_with_parser<std::endian::little>(), true));
    expect(eq(nested_agrees_with_parser<std::endian::big>(), true));
    auto le = s2s::struct_view_le<nested_record>(std::span<const u8, nested_width>{nested_sample});
    auto be = s2s::struct_view_be<nested_record>(std::span<const u8, nested_width>{nested_sample});
    expect(eq((*le)["mid"_f]["hdr"_f]["len"_f], 0x85040302u));
    expect(eq((*be)["mid"_f]["hdr"_f]["len"_f], 0x02030485u));
    expect(eq((*le)["mid"_f]["hdr"_f]["narrow"_f], 0x090807u));
    expect(eq((*le)["tail"_f], u16{0x0d0c}));
  };

  "the width and every offset through nesting are compile-time constants"_test = [] constexpr {
    expect(eq(s2s::wire_width<nested_record>, nested_width));
    expect(eq(s2s::wire_width<middle>, std::size_t{10}));
    constexpr auto v = s2s::struct_view_le<nested_record>(std::span<const u8, nested_width>{nested_sample});
    static_assert(v.has_value());
    static_assert((*v)["mid"_f]["hdr"_f]["kind"_f] == 1);
  };

  "a sub-view is copyable, non-owning and the same type as a top-level view"_test = [] constexpr {
    auto view = s2s::struct_view_le<nested_record>(std::span<const u8>{nested_sample});
    auto sub = (*view)["mid"_f];
    auto copy = sub;
    expect(eq(copy["m"_f] == sub["m"_f], true));
    expect(eq(sizeof(sub) == sizeof(const u8*), true));
    expect(eq(std::is_trivially_copyable_v<decltype(sub)>, true));
    using middle_view = std::remove_cvref_t<decltype(*s2s::struct_view_le<middle>(std::span<const u8>{}))>;
    expect(eq(std::is_same_v<std::remove_cvref_t<decltype(sub)>, middle_view>, true));
    expect(eq(std::is_const_v<decltype((*view)["mid"_f])>, true));
  };

  "reading a nested field leaves its siblings and the enclosing record unread"_test = [] constexpr {
    auto original = nested_sample;
    auto mutated = nested_sample;
    mutated[0] = 0x55;
    mutated[1] = 0x66;
    mutated[3] = 0x77;
    mutated[12] = 0x88;
    auto before = s2s::struct_view_le<nested_record>(std::span<const u8>{original});
    auto after = s2s::struct_view_le<nested_record>(std::span<const u8>{mutated});
    expect(eq(before.has_value() && after.has_value(), true));
    expect(eq((*before)["mid"_f]["hdr"_f]["len"_f] == (*after)["mid"_f]["hdr"_f]["len"_f], true));
    expect(eq((*before)["mid"_f]["hdr"_f]["kind"_f] == (*after)["mid"_f]["hdr"_f]["kind"_f], false));
  };

  "a short span names the outermost field, like the parser"_test = [] constexpr {
    auto view = s2s::struct_view_le<nested_record>(std::span<const u8>{nested_sample}.first(nested_width - 1));
    expect(eq(view.has_value(), false));
    expect(eq(view.error().failure_reason, s2s::error_reason::buffer_exhaustion));
    expect(eq(view.error().failed_at == "tail", true));
    auto shorter = s2s::struct_view_le<nested_record>(std::span<const u8>{nested_sample}.first(6));
    expect(eq(shorter.error().failed_at == "mid", true));
  };

  "a constrained field inside a nested record is checked at creation"_test = [] constexpr {
    expect(eq(s2s::any_field_constrained_v<field_checked>, true));
    std::array<u8, 7> good{0x01, 0x01, 0x07, 0x00, 0x00, 0x00, 0x01};
    expect(eq(creation_agrees_with_parser<field_checked, std::endian::little>(good), true));
    expect(eq(s2s::struct_view_le<field_checked>(std::span<const u8>{good}).has_value(), true));
    auto bad_len = good;
    bad_len[2] = 0x06;
    expect(eq(creation_agrees_with_parser<field_checked, std::endian::little>(bad_len), true));
    auto err = s2s::struct_view_le<field_checked>(std::span<const u8>{bad_len}).error();
    expect(eq(err.failure_reason, s2s::error_reason::validation_failure));
    expect(eq(err.failed_at == "hdr", true));
    auto bad_both = bad_len;
    bad_both[6] = 0x02;
    expect(eq(s2s::struct_view_le<field_checked>(std::span<const u8>{bad_both}).error().failed_at == "hdr", true));
    auto bad_post = good;
    bad_post[6] = 0x02;
    expect(eq(creation_agrees_with_parser<field_checked, std::endian::little>(bad_post), true));
    expect(eq(s2s::struct_view_le<field_checked>(std::span<const u8>{bad_post}).error().failed_at == "post", true));
    std::array<u8, 7> be_good{0x01, 0x01, 0x00, 0x00, 0x00, 0x07, 0x01};
    expect(eq(creation_agrees_with_parser<field_checked, std::endian::big>(be_good), true));
  };

  "a constraint on the nested record itself is checked at creation"_test = [] constexpr {
    expect(eq(s2s::any_field_constrained_v<record_checked>, true));
    std::array<u8, 6> good{0x01, 0x07, 0x07, 0x00, 0x00, 0x00};
    expect(eq(s2s::struct_view_le<record_checked>(std::span<const u8>{good}).has_value(), true));
    expect(eq(creation_agrees_with_parser<record_checked, std::endian::little>(good), true));
    auto mismatch = good;
    mismatch[1] = 0x08;
    expect(eq(creation_agrees_with_parser<record_checked, std::endian::little>(mismatch), true));
    auto err = s2s::struct_view_le<record_checked>(std::span<const u8>{mismatch}).error();
    expect(eq(err.failure_reason, s2s::error_reason::validation_failure));
    expect(eq(err.failed_at == "hdr", true));
    auto inner_bad = good;
    inner_bad[2] = 0x06;
    inner_bad[1] = 0x06;
    expect(eq(creation_agrees_with_parser<record_checked, std::endian::little>(inner_bad), true));
    expect(eq(creation_agrees_with_parser<record_checked, std::endian::big>(good), true));
  };

  "an array of records indexes as sub-views that match the parser in both orders"_test = [] constexpr {
    expect(eq(table_agrees_with_parser<std::endian::little>(), true));
    expect(eq(table_agrees_with_parser<std::endian::big>(), true));
    auto le = s2s::struct_view_le<table_record>(std::span<const u8, table_width>{table_sample});
    auto be = s2s::struct_view_be<table_record>(std::span<const u8, table_width>{table_sample});
    expect(eq((*le)["entries"_f][1]["len"_f], 0x33323130u));
    expect(eq((*be)["entries"_f][1]["len"_f], 0x30313233u));
    expect(eq((*le)["entries"_f][2]["flags"_f], u16{0x6160}));
    expect(eq((*le)["entries"_f].size(), std::size_t{3}));
  };

  "the width and every element offset are compile-time constants"_test = [] constexpr {
    expect(eq(s2s::wire_width<table_record>, table_width));
    expect(eq(s2s::record_layout<table_record>::offsets[2], std::size_t{1 + 21}));
    constexpr auto v = s2s::struct_view_le<table_record>(std::span<const u8, table_width>{table_sample});
    constexpr auto flags = (*v)["entries"_f][2]["flags"_f];
    expect(eq(flags, u16{0x6160}));
  };

  "an element is a sub-view of the same type as a top-level view"_test = [] constexpr {
    auto view = s2s::struct_view_le<table_record>(std::span<const u8>{table_sample});
    auto entries = (*view)["entries"_f];
    auto entry = entries[1];
    expect(eq(sizeof(entries) == sizeof(const u8*), true));
    expect(eq(sizeof(entry) == sizeof(const u8*), true));
    expect(eq(std::is_trivially_copyable_v<decltype(entries)>, true));
    expect(eq(entry["kind"_f], u8{2}));
  };

  "reading one element leaves the other elements unread"_test = [] constexpr {
    auto original = table_sample;
    auto mutated = table_sample;
    mutated[2] = 0x99;
    mutated[16] = 0x99;
    auto before = s2s::struct_view_le<table_record>(std::span<const u8>{original});
    auto after = s2s::struct_view_le<table_record>(std::span<const u8>{mutated});
    expect(eq((*before)["entries"_f][1]["len"_f] == (*after)["entries"_f][1]["len"_f], true));
    expect(eq((*before)["entries"_f][0]["len"_f] == (*after)["entries"_f][0]["len"_f], false));
  };

  "a span short of the table names the array field, like the parser"_test = [] constexpr {
    auto view = s2s::struct_view_le<table_record>(std::span<const u8>{table_sample}.first(table_width - 1));
    expect(eq(view.has_value(), false));
    expect(eq(view.error().failed_at == "tail", true));
    auto shorter = s2s::struct_view_le<table_record>(std::span<const u8>{table_sample}.first(10));
    expect(eq(shorter.error().failed_at == "entries", true));
  };

  "arrays nested in records nested in arrays read through chained indexing"_test = [] constexpr {
    static_assert(s2s::fixed_layout_schema<groups_record>);
    expect(eq(s2s::wire_width<groups_record>, groups_width));
    auto bytes = groups_sample;
    memstream<groups_width> stream(bytes);
    auto parsed = s2s::struct_cast_le<groups_record>(stream);
    auto viewed = s2s::struct_view_le<groups_record>(std::span<const u8>{groups_sample});
    expect(eq(parsed.has_value() && viewed.has_value(), true));
    for(std::size_t g = 0; g < 2; ++g) {
      expect(eq((*parsed)["groups"_f][g]["n"_f] == (*viewed)["groups"_f][g]["n"_f], true));
      for(std::size_t l = 0; l < 2; ++l) {
        expect(eq((*parsed)["groups"_f][g]["leaves"_f][l]["id"_f] ==
                  (*viewed)["groups"_f][g]["leaves"_f][l]["id"_f], true));
        expect(eq((*parsed)["groups"_f][g]["leaves"_f][l]["size"_f] ==
                  (*viewed)["groups"_f][g]["leaves"_f][l]["size"_f], true));
      }
    }
    expect(eq((*viewed)["groups"_f][1]["leaves"_f][1]["size"_f], u16{8}));
  };

  "a constraint in any element is checked at creation, in wire order"_test = [] constexpr {
    expect(eq(s2s::any_field_constrained_v<groups_record>, true));
    expect(eq(s2s::any_field_constrained_v<table_record>, false));
    expect(eq(creation_agrees_with_parser<groups_record, std::endian::little>(groups_sample), true));
    expect(eq(s2s::struct_view_le<groups_record>(std::span<const u8>{groups_sample}).has_value(), true));
    for(std::size_t at: {3uz, 13uz}) {
      auto bad = groups_sample;
      bad[at] = 0x04;
      expect(eq(creation_agrees_with_parser<groups_record, std::endian::little>(bad), true));
      auto err = s2s::struct_view_le<groups_record>(std::span<const u8>{bad}).error();
      expect(eq(err.failure_reason, s2s::error_reason::validation_failure));
      expect(eq(err.failed_at == "groups", true));
    }
  };

  "a constraint on the array itself is checked at creation"_test = [] constexpr {
    expect(eq(s2s::any_field_constrained_v<array_checked>, true));
    std::array<u8, array_checked_width> good{};
    good[2] = 32;
    good[9] = 32;
    good[16] = 32;
    expect(eq(creation_agrees_with_parser<array_checked, std::endian::little>(good), true));
    expect(eq(s2s::struct_view_le<array_checked>(std::span<const u8>{good}).has_value(), true));
    auto bad = good;
    bad[16] = 33;
    expect(eq(creation_agrees_with_parser<array_checked, std::endian::little>(bad), true));
    auto err = s2s::struct_view_le<array_checked>(std::span<const u8>{bad}).error();
    expect(eq(err.failure_reason, s2s::error_reason::validation_failure));
    expect(eq(err.failed_at == "entries", true));
    std::array<u8, array_checked_width> be_good{};
    be_good[4] = 32;
    be_good[11] = 32;
    be_good[18] = 32;
    expect(eq(creation_agrees_with_parser<array_checked, std::endian::big>(be_good), true));
  };
}
