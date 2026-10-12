#include <gtest/gtest.h>
#include <array>
#include <vector>
#include <span>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

namespace {
using i16 = short;

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
constexpr std::array<u8, record_width> sample{
  0x7f,
  0x01, 0x02, 0x03, 0x84,
  0x05, 0x86,
  0x07, 0x08, 0x09,
  0x00, 0x00, 0x80, 0x3f,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x3f,
  0xfe, 0xff
};

void write_sample(std::size_t trailing = 0) {
  std::ofstream file("test_input.bin", std::ios::out | std::ios::binary);
  file.write(reinterpret_cast<const char*>(sample.data()), sample.size());
  for(std::size_t idx = 0; idx < trailing; ++idx)
    file.put(static_cast<char>(0xaa));
}

auto slurp() -> std::vector<char> {
  std::ifstream file("test_input.bin", std::ios::in | std::ios::binary);
  return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

template <typename parsed_t, typename view_t>
void expect_same_fields(const parsed_t& p, const view_t& v) {
  EXPECT_EQ(p["a"_f], v["a"_f]);
  EXPECT_EQ(p["b"_f], v["b"_f]);
  EXPECT_EQ(p["c"_f], v["c"_f]);
  EXPECT_EQ(p["narrow"_f], v["narrow"_f]);
  EXPECT_EQ(p["f"_f], v["f"_f]);
  EXPECT_EQ(p["d"_f], v["d"_f]);
  EXPECT_EQ(p["s"_f], v["s"_f]);
}

using aggregates =
  s2s::struct_field_list<
    s2s::basic_field<"a", u8, 1_B>,
    s2s::fixed_array_field<"arr", u16, 3>,
    s2s::c_arr_field<"carr", u32, 2>,
    s2s::c_str_field<"cs", 3>,
    s2s::fixed_string_field<"fs", 4>
  >;
constexpr std::size_t aggregates_width = 24;
constexpr std::array<u8, aggregates_width> aggregates_sample{
  0x7f,
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
  0x11, 0x12, 0x13, 0x14, 0x21, 0x22, 0x23, 0x24,
  'a', 'b', 'c', 0,
  'w', 'x', 'y', 'z', 0
};

void write_aggregates() {
  std::ofstream file("test_input.bin", std::ios::out | std::ios::binary);
  file.write(reinterpret_cast<const char*>(aggregates_sample.data()), aggregates_sample.size());
}

template <typename parsed_t, typename view_t>
void expect_same_aggregates(const parsed_t& p, const view_t& v) {
  EXPECT_EQ(p["a"_f], v["a"_f]);
  EXPECT_EQ(p["arr"_f], v["arr"_f]);
  EXPECT_EQ(p["fs"_f], v["fs"_f]);
  const auto carr = v["carr"_f];
  for(std::size_t idx = 0; idx < carr.size(); ++idx)
    EXPECT_EQ(p["carr"_f][idx], carr[idx]);
  const auto cs = v["cs"_f];
  for(std::size_t idx = 0; idx < cs.size(); ++idx)
    EXPECT_EQ(p["cs"_f][idx], cs[idx]);
}
}

TEST(StructViewRead, MatchesTheParserLittleEndianOverADynamicSpan) {
  write_sample();
  std::ifstream file("test_input.bin", std::ios::in | std::ios::binary);
  auto parsed = s2s::struct_cast_le<record>(file);
  auto bytes = slurp();
  auto viewed = s2s::struct_view_le<record>(std::span<const char>{bytes});
  ASSERT_TRUE(parsed.has_value());
  ASSERT_TRUE(viewed.has_value());
  expect_same_fields(*parsed, *viewed);
}

TEST(StructViewRead, MatchesTheParserBigEndianOverADynamicSpan) {
  write_sample();
  std::ifstream file("test_input.bin", std::ios::in | std::ios::binary);
  auto parsed = s2s::struct_cast_be<record>(file);
  auto bytes = slurp();
  auto viewed = s2s::struct_view_be<record>(std::span<const char>{bytes});
  ASSERT_TRUE(parsed.has_value());
  ASSERT_TRUE(viewed.has_value());
  expect_same_fields(*parsed, *viewed);
}

TEST(StructViewRead, MatchesTheParserOverAStaticSpan) {
  write_sample();
  auto bytes = slurp();
  std::array<char, record_width> fixed{};
  std::copy(bytes.begin(), bytes.end(), fixed.begin());
  std::ifstream le_file("test_input.bin", std::ios::in | std::ios::binary);
  std::ifstream be_file("test_input.bin", std::ios::in | std::ios::binary);
  auto parsed_le = s2s::struct_cast_le<record>(le_file);
  auto parsed_be = s2s::struct_cast_be<record>(be_file);
  auto viewed_le = s2s::struct_view_le<record>(std::span<const char, record_width>{fixed});
  auto viewed_be = s2s::struct_view_be<record>(std::span<char, record_width>{fixed});
  ASSERT_TRUE(viewed_le.has_value());
  ASSERT_TRUE(viewed_be.has_value());
  expect_same_fields(*parsed_le, *viewed_le);
  expect_same_fields(*parsed_be, *viewed_be);
}

TEST(StructViewRead, IgnoresBytesPastTheRecord) {
  write_sample(7);
  auto bytes = slurp();
  ASSERT_EQ(bytes.size(), record_width + 7);
  auto viewed = s2s::struct_view_le<record>(std::span<const char>{bytes});
  ASSERT_TRUE(viewed.has_value());
  EXPECT_EQ((*viewed)["b"_f], 0x84030201u);
  EXPECT_EQ((*viewed)["s"_f], -2);
}

TEST(StructViewRead, ReadsFromAnOddAddressAndMisalignedOffsets) {
  alignas(8) std::array<char, record_width + 1> storage{};
  for(std::size_t idx = 0; idx < record_width; ++idx)
    storage[idx + 1] = static_cast<char>(sample[idx]);
  std::span<const char> at_odd{storage.data() + 1, record_width};
  ASSERT_NE(reinterpret_cast<std::uintptr_t>(at_odd.data()) % 2, 0u);

  auto le = s2s::struct_view_le<record>(at_odd);
  auto be = s2s::struct_view_be<record>(at_odd);
  ASSERT_TRUE(le.has_value());
  ASSERT_TRUE(be.has_value());
  EXPECT_EQ((*le)["b"_f], 0x84030201u);
  EXPECT_EQ((*be)["b"_f], 0x01020384u);
  EXPECT_EQ((*le)["c"_f], 0x8605);
  EXPECT_EQ((*be)["c"_f], 0x0586);
  EXPECT_EQ((*le)["d"_f], 1.0);
}

TEST(StructViewRead, AShortDynamicSpanFailsLikeTheParser) {
  write_sample();
  auto bytes = slurp();
  bytes.pop_back();
  std::string truncated(bytes.begin(), bytes.end());
  std::istringstream short_stream(truncated, std::ios::in | std::ios::binary);
  auto parsed = s2s::struct_cast_le<record>(short_stream);
  auto viewed = s2s::struct_view_le<record>(std::span<const char>{bytes});
  ASSERT_FALSE(parsed.has_value());
  ASSERT_FALSE(viewed.has_value());
  EXPECT_EQ(viewed.error().failure_reason, s2s::error_reason::buffer_exhaustion);
  EXPECT_EQ(parsed.error().failure_reason, viewed.error().failure_reason);
  EXPECT_EQ(parsed.error().failed_at, viewed.error().failed_at);
}

TEST(StructViewRead, ChangingAnUnreadFieldDoesNotAffectOtherReads) {
  write_sample();
  auto bytes = slurp();
  auto changed = bytes;
  changed[14] = 0x66;
  auto before = s2s::struct_view_le<record>(std::span<const char>{bytes});
  auto after = s2s::struct_view_le<record>(std::span<const char>{changed});
  ASSERT_TRUE(before.has_value());
  ASSERT_TRUE(after.has_value());
  EXPECT_EQ((*before)["b"_f], (*after)["b"_f]);
  EXPECT_NE((*before)["d"_f], (*after)["d"_f]);
}

TEST(StructViewRead, WireWidthIsTheSumOfTheDeclaredWidths) {
  static_assert(s2s::wire_width<record> == record_width);
  EXPECT_EQ(s2s::wire_width<record>, record_width);
}

TEST(StructViewRead, AggregatesMatchTheParserInBothOrders) {
  write_aggregates();
  auto bytes = slurp();
  std::ifstream le_file("test_input.bin", std::ios::in | std::ios::binary);
  std::ifstream be_file("test_input.bin", std::ios::in | std::ios::binary);
  auto parsed_le = s2s::struct_cast_le<aggregates>(le_file);
  auto parsed_be = s2s::struct_cast_be<aggregates>(be_file);
  auto viewed_le = s2s::struct_view_le<aggregates>(std::span<const char>{bytes});
  auto viewed_be = s2s::struct_view_be<aggregates>(std::span<const char>{bytes});
  ASSERT_TRUE(parsed_le.has_value());
  ASSERT_TRUE(parsed_be.has_value());
  ASSERT_TRUE(viewed_le.has_value());
  ASSERT_TRUE(viewed_be.has_value());
  expect_same_aggregates(*parsed_le, *viewed_le);
  expect_same_aggregates(*parsed_be, *viewed_be);
  EXPECT_EQ((*viewed_be)["arr"_f], (std::array<u16, 3>{0x0102, 0x0304, 0x0506}));
  EXPECT_EQ((*viewed_le)["fs"_f].to_sv(), "wxyz");
}

TEST(StructViewRead, AggregatesReadFromAnOddAddress) {
  alignas(8) std::array<char, aggregates_width + 1> storage{};
  for(std::size_t idx = 0; idx < aggregates_width; ++idx)
    storage[idx + 1] = static_cast<char>(aggregates_sample[idx]);
  std::span<const char> at_odd{storage.data() + 1, aggregates_width};
  auto le = s2s::struct_view_le<aggregates>(at_odd);
  auto be = s2s::struct_view_be<aggregates>(at_odd);
  ASSERT_TRUE(le.has_value());
  ASSERT_TRUE(be.has_value());
  EXPECT_EQ((*le)["carr"_f], (std::array<u32, 2>{0x14131211, 0x24232221}));
  EXPECT_EQ((*be)["carr"_f], (std::array<u32, 2>{0x11121314, 0x21222324}));
  EXPECT_EQ((*be)["cs"_f], (std::array<char, 4>{'a', 'b', 'c', 0}));
}

namespace {
using magic_record =
  s2s::struct_field_list<
    s2s::basic_field<"lead", u8, 1_B>,
    s2s::magic_number<"num", u32, 4_B, 0xdeadbeef>,
    s2s::magic_string<"str", "GIF">,
    s2s::magic_byte_array<"arr", 2, std::array<unsigned char, 2>{0xca, 0xfe}>,
    s2s::basic_field<"v", u16, 2_B, s2s::any_of(static_cast<u16>(0x0201), static_cast<u16>(0x0102))>
  >;

constexpr std::array<u8, 13> magic_sample{
  0x42,
  0xef, 0xbe, 0xad, 0xde,
  'G', 'I', 'F', 0,
  0xca, 0xfe,
  0x01, 0x02
};

void write_magic(const std::array<u8, 13>& bytes) {
  std::ofstream file("test_input.bin", std::ios::out | std::ios::binary);
  file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

void write_and_compare(std::array<u8, 13> bytes) {
  write_magic(bytes);
  auto data = slurp();
  std::ifstream le_file("test_input.bin", std::ios::in | std::ios::binary);
  std::ifstream be_file("test_input.bin", std::ios::in | std::ios::binary);
  auto parsed_le = s2s::struct_cast_le<magic_record>(le_file);
  auto parsed_be = s2s::struct_cast_be<magic_record>(be_file);
  auto viewed_le = s2s::struct_view_le<magic_record>(std::span<const char>{data});
  auto viewed_be = s2s::struct_view_be<magic_record>(std::span<const char>{data});
  ASSERT_EQ(parsed_le.has_value(), viewed_le.has_value());
  ASSERT_EQ(parsed_be.has_value(), viewed_be.has_value());
  if(!parsed_le) {
    EXPECT_EQ(parsed_le.error().failure_reason, viewed_le.error().failure_reason);
    EXPECT_EQ(parsed_le.error().failed_at, viewed_le.error().failed_at);
  }
  if(!parsed_be) {
    EXPECT_EQ(parsed_be.error().failure_reason, viewed_be.error().failure_reason);
    EXPECT_EQ(parsed_be.error().failed_at, viewed_be.error().failed_at);
  }
}
}

TEST(StructViewRead, MagicFieldsReadBackLikeTheParser) {
  write_magic(magic_sample);
  auto bytes = slurp();
  std::ifstream file("test_input.bin", std::ios::in | std::ios::binary);
  auto parsed = s2s::struct_cast_le<magic_record>(file);
  auto viewed = s2s::struct_view_le<magic_record>(std::span<const char>{bytes});
  ASSERT_TRUE(parsed.has_value());
  ASSERT_TRUE(viewed.has_value());
  EXPECT_EQ((*parsed)["num"_f], (*viewed)["num"_f]);
  EXPECT_EQ((*parsed)["str"_f], (*viewed)["str"_f]);
  EXPECT_EQ((*parsed)["arr"_f], (*viewed)["arr"_f]);
  EXPECT_EQ((*viewed)["str"_f].to_sv(), "GIF");
  static_assert(std::is_same_v<std::remove_cvref_t<decltype((*viewed)["str"_f])>, s2s::fixed_string<3>>);
}

TEST(StructViewRead, CreationFailsExactlyWhereTheParserDoes) {
  write_and_compare(magic_sample);
  auto bad_num = magic_sample;
  bad_num[2] = 0;
  write_and_compare(bad_num);
  auto bad_str = magic_sample;
  bad_str[5] = 'X';
  write_and_compare(bad_str);
  auto bad_arr = magic_sample;
  bad_arr[9] = 0;
  write_and_compare(bad_arr);
  auto bad_v = magic_sample;
  bad_v[11] = 0x09;
  write_and_compare(bad_v);
}

TEST(StructViewRead, AShortSpanFailsWithExhaustionBeforeAConstraint) {
  auto bad = magic_sample;
  bad[1] = 0;
  write_magic(bad);
  auto bytes = slurp();
  bytes.pop_back();
  auto viewed = s2s::struct_view_le<magic_record>(std::span<const char>{bytes});
  ASSERT_FALSE(viewed.has_value());
  EXPECT_EQ(viewed.error().failure_reason, s2s::error_reason::buffer_exhaustion);
}

TEST(StructViewRead, ConstrainedFieldsAreCheckedAtAnOddAddress) {
  alignas(8) std::array<char, magic_sample.size() + 1> storage{};
  for(std::size_t idx = 0; idx < magic_sample.size(); ++idx)
    storage[idx + 1] = static_cast<char>(magic_sample[idx]);
  std::span<const char> at_odd{storage.data() + 1, magic_sample.size()};
  auto le = s2s::struct_view_le<magic_record>(at_odd);
  ASSERT_TRUE(le.has_value());
  EXPECT_EQ((*le)["num"_f], 0xdeadbeefu);
  EXPECT_FALSE(s2s::struct_view_be<magic_record>(at_odd).has_value());
}

TEST(StructViewRead, ChangingAnUnconstrainedUnreadFieldChangesNothing) {
  write_magic(magic_sample);
  auto bytes = slurp();
  auto changed = bytes;
  changed[0] = static_cast<char>(0x99);
  auto before = s2s::struct_view_le<magic_record>(std::span<const char>{bytes});
  auto after = s2s::struct_view_le<magic_record>(std::span<const char>{changed});
  ASSERT_TRUE(before.has_value());
  ASSERT_TRUE(after.has_value());
  EXPECT_EQ((*before)["num"_f], (*after)["num"_f]);
  EXPECT_EQ((*before)["v"_f], (*after)["v"_f]);
  EXPECT_NE((*before)["lead"_f], (*after)["lead"_f]);
}

namespace {
using inner_header =
  s2s::struct_field_list<
    s2s::basic_field<"kind", u8, 1_B>,
    s2s::basic_field<"len", u32, 4_B, s2s::gte(7u)>,
    s2s::basic_field<"narrow", u32, 3_B>
  >;
constexpr auto kind_below_len = [](const inner_header& h) { return h["kind"_f] < h["len"_f]; };
using middle =
  s2s::struct_field_list<
    s2s::basic_field<"m", u16, 2_B>,
    s2s::struct_field<"hdr", inner_header, kind_below_len>
  >;
using nested_record =
  s2s::struct_field_list<
    s2s::basic_field<"lead", u8, 1_B>,
    s2s::struct_field<"mid", middle>,
    s2s::basic_field<"tail", u16, 2_B>
  >;
constexpr std::size_t nested_width = 1 + (2 + (1 + 4 + 3)) + 2;
constexpr std::array<u8, nested_width> nested_sample{
  0x7f,
  0x0a, 0x0b,
  0x01,
  0x07, 0x00, 0x00, 0x00,
  0x07, 0x08, 0x09,
  0x0c, 0x0d
};

void write_nested(const std::array<u8, nested_width>& bytes) {
  std::ofstream file("test_input.bin", std::ios::out | std::ios::binary);
  file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

template <typename parsed_t, typename view_t>
void expect_same_nested(const parsed_t& p, const view_t& v) {
  EXPECT_EQ(p["lead"_f], v["lead"_f]);
  EXPECT_EQ(p["tail"_f], v["tail"_f]);
  EXPECT_EQ(p["mid"_f]["m"_f], v["mid"_f]["m"_f]);
  EXPECT_EQ(p["mid"_f]["hdr"_f]["kind"_f], v["mid"_f]["hdr"_f]["kind"_f]);
  EXPECT_EQ(p["mid"_f]["hdr"_f]["len"_f], v["mid"_f]["hdr"_f]["len"_f]);
  EXPECT_EQ(p["mid"_f]["hdr"_f]["narrow"_f], v["mid"_f]["hdr"_f]["narrow"_f]);
}
}

TEST(StructViewRead, ANestedRecordReadsAsASubViewInBothOrders) {
  write_nested(nested_sample);
  auto bytes = slurp();
  std::ifstream le_file("test_input.bin", std::ios::in | std::ios::binary);
  std::ifstream be_file("test_input.bin", std::ios::in | std::ios::binary);
  auto parsed_le = s2s::struct_cast_le<nested_record>(le_file);
  auto parsed_be = s2s::struct_cast_be<nested_record>(be_file);
  auto viewed_le = s2s::struct_view_le<nested_record>(std::span<const char>{bytes});
  auto viewed_be = s2s::struct_view_be<nested_record>(std::span<const char>{bytes});
  ASSERT_EQ(parsed_le.has_value(), viewed_le.has_value());
  ASSERT_TRUE(viewed_le.has_value());
  ASSERT_TRUE(parsed_be.has_value() == viewed_be.has_value());
  expect_same_nested(*parsed_le, *viewed_le);
  EXPECT_EQ((*viewed_le)["mid"_f]["hdr"_f]["len"_f], 7u);
  EXPECT_EQ((*viewed_le)["mid"_f]["hdr"_f]["narrow"_f], 0x090807u);
  static_assert(s2s::wire_width<nested_record> == nested_width);
}

TEST(StructViewRead, ASubViewReadsFromAnOddAddress) {
  alignas(8) std::array<char, nested_width + 1> storage{};
  for(std::size_t idx = 0; idx < nested_width; ++idx)
    storage[idx + 1] = static_cast<char>(nested_sample[idx]);
  std::span<const char> at_odd{storage.data() + 1, nested_width};
  auto le = s2s::struct_view_le<nested_record>(at_odd);
  ASSERT_TRUE(le.has_value());
  auto hdr = (*le)["mid"_f]["hdr"_f];
  EXPECT_EQ(hdr["len"_f], 7u);
  EXPECT_EQ(hdr["narrow"_f], 0x090807u);
  auto copy = hdr;
  EXPECT_EQ(copy["kind"_f], 1);
  EXPECT_EQ(sizeof(hdr), sizeof(const char*));
}

TEST(StructViewRead, NestedConstraintsFailLikeTheParser) {
  auto check = [](std::array<u8, nested_width> bytes) {
    write_nested(bytes);
    auto data = slurp();
    std::ifstream file("test_input.bin", std::ios::in | std::ios::binary);
    auto parsed = s2s::struct_cast_le<nested_record>(file);
    auto viewed = s2s::struct_view_le<nested_record>(std::span<const char>{data});
    ASSERT_EQ(parsed.has_value(), viewed.has_value());
    if(!parsed) {
      EXPECT_EQ(parsed.error().failure_reason, viewed.error().failure_reason);
      EXPECT_EQ(parsed.error().failed_at, viewed.error().failed_at);
      EXPECT_EQ(viewed.error().failed_at, "mid");
    }
  };
  check(nested_sample);
  auto bad_field = nested_sample;
  bad_field[4] = 0x06;
  check(bad_field);
  auto bad_record = nested_sample;
  bad_record[3] = 0x09;
  check(bad_record);
}

TEST(StructViewRead, AShortSpanNamesTheOutermostFieldOfANestedRecord) {
  write_nested(nested_sample);
  auto bytes = slurp();
  bytes.resize(6);
  auto viewed = s2s::struct_view_le<nested_record>(std::span<const char>{bytes});
  ASSERT_FALSE(viewed.has_value());
  EXPECT_EQ(viewed.error().failure_reason, s2s::error_reason::buffer_exhaustion);
  EXPECT_EQ(viewed.error().failed_at, "mid");
}

namespace {
using table_entry =
  s2s::struct_field_list<
    s2s::basic_field<"kind", u8, 1_B>,
    s2s::basic_field<"len", u32, 4_B, s2s::gte(0u)>,
    s2s::basic_field<"flags", u16, 2_B>
  >;
using table_record =
  s2s::struct_field_list<
    s2s::basic_field<"lead", u8, 1_B>,
    s2s::array_of_records<"entries", table_entry, 3>,
    s2s::basic_field<"tail", u16, 2_B>
  >;
constexpr std::size_t table_width = 1 + 3 * 7 + 2;
constexpr std::array<u8, table_width> table_sample{
  0x7f,
  0x01, 0x10, 0x11, 0x12, 0x13, 0x20, 0x21,
  0x02, 0x30, 0x31, 0x32, 0x33, 0x40, 0x41,
  0x03, 0x50, 0x51, 0x52, 0x53, 0x60, 0x61,
  0x0c, 0x0d
};

using checked_entry =
  s2s::struct_field_list<
    s2s::basic_field<"kind", u8, 1_B>,
    s2s::basic_field<"len", u16, 2_B, s2s::lte(u16{9})>
  >;
using checked_table =
  s2s::struct_field_list<
    s2s::array_of_records<"entries", checked_entry, 3>
  >;

void write_bytes(const auto& bytes) {
  std::ofstream file("test_input.bin", std::ios::out | std::ios::binary);
  file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}
}

TEST(StructViewRead, AnArrayOfRecordsIndexesAsSubViewsInBothOrders) {
  write_bytes(table_sample);
  auto bytes = slurp();
  std::ifstream le_file("test_input.bin", std::ios::in | std::ios::binary);
  std::ifstream be_file("test_input.bin", std::ios::in | std::ios::binary);
  auto parsed_le = s2s::struct_cast_le<table_record>(le_file);
  auto parsed_be = s2s::struct_cast_be<table_record>(be_file);
  auto viewed_le = s2s::struct_view_le<table_record>(std::span<const char>{bytes});
  auto viewed_be = s2s::struct_view_be<table_record>(std::span<const char>{bytes});
  ASSERT_TRUE(parsed_le && parsed_be && viewed_le && viewed_be);
  for(std::size_t idx = 0; idx < 3; ++idx) {
    EXPECT_EQ((*parsed_le)["entries"_f][idx]["len"_f], (*viewed_le)["entries"_f][idx]["len"_f]);
    EXPECT_EQ((*parsed_le)["entries"_f][idx]["flags"_f], (*viewed_le)["entries"_f][idx]["flags"_f]);
    EXPECT_EQ((*parsed_be)["entries"_f][idx]["len"_f], (*viewed_be)["entries"_f][idx]["len"_f]);
    EXPECT_EQ((*parsed_be)["entries"_f][idx]["kind"_f], (*viewed_be)["entries"_f][idx]["kind"_f]);
  }
  EXPECT_EQ((*viewed_le)["entries"_f].size(), 3u);
  static_assert(s2s::wire_width<table_record> == table_width);
}

TEST(StructViewRead, AnElementReadsFromAnOddAddress) {
  alignas(8) std::array<char, table_width + 1> storage{};
  for(std::size_t idx = 0; idx < table_width; ++idx)
    storage[idx + 1] = static_cast<char>(table_sample[idx]);
  auto le = s2s::struct_view_le<table_record>(std::span<const char>{storage.data() + 1, table_width});
  ASSERT_TRUE(le.has_value());
  EXPECT_EQ((*le)["entries"_f][2]["len"_f], 0x53525150u);
  EXPECT_EQ(sizeof((*le)["entries"_f][1]), sizeof(const char*));
}

TEST(StructViewRead, AConstraintInAnyElementFailsLikeTheParser) {
  auto check = [](std::array<u8, 9> bytes, bool ok) {
    write_bytes(bytes);
    auto data = slurp();
    std::ifstream file("test_input.bin", std::ios::in | std::ios::binary);
    auto parsed = s2s::struct_cast_le<checked_table>(file);
    auto viewed = s2s::struct_view_le<checked_table>(std::span<const char>{data});
    ASSERT_EQ(parsed.has_value(), viewed.has_value());
    ASSERT_EQ(viewed.has_value(), ok);
    if(!parsed) {
      EXPECT_EQ(parsed.error().failure_reason, viewed.error().failure_reason);
      EXPECT_EQ(parsed.error().failed_at, viewed.error().failed_at);
      EXPECT_EQ(viewed.error().failed_at, "entries");
    }
  };
  check({1, 9, 0, 2, 9, 0, 3, 9, 0}, true);
  check({1, 10, 0, 2, 9, 0, 3, 9, 0}, false);
  check({1, 9, 0, 2, 9, 0, 3, 10, 0}, false);
}

TEST(StructViewRead, MisalignedFieldsReadInBothOrdersAtDepthFromAnOddAddress) {
  alignas(8) std::array<char, table_width + 1> storage{};
  for(std::size_t idx = 0; idx < table_width; ++idx)
    storage[idx + 1] = static_cast<char>(table_sample[idx]);
  std::span<const char> at_odd{storage.data() + 1, table_width};
  auto be = s2s::struct_view_be<table_record>(at_odd);
  ASSERT_TRUE(be.has_value());
  EXPECT_EQ((*be)["entries"_f][1]["len"_f], 0x30313233u);
  EXPECT_EQ((*be)["entries"_f][1]["flags"_f], 0x4041);
  EXPECT_EQ((*be)["tail"_f], 0x0c0d);

  alignas(8) std::array<char, nested_width + 1> nested_storage{};
  for(std::size_t idx = 0; idx < nested_width; ++idx)
    nested_storage[idx + 1] = static_cast<char>(nested_sample[idx]);
  auto nested_be = s2s::struct_view_be<nested_record>(
    std::span<const char>{nested_storage.data() + 1, nested_width});
  ASSERT_TRUE(nested_be.has_value());
  EXPECT_EQ((*nested_be)["mid"_f]["m"_f], 0x0a0b);
  EXPECT_EQ((*nested_be)["mid"_f]["hdr"_f]["len"_f], 0x07000000u);
  EXPECT_EQ((*nested_be)["mid"_f]["hdr"_f]["narrow"_f], 0x070809u);
  EXPECT_EQ((*nested_be)["tail"_f], 0x0c0d);
}

TEST(StructViewRead, ChangingUnreadBytesAtDepthChangesNothingElse) {
  auto expect_untouched = [](auto create) {
    auto nested_changed = nested_sample;
    for(std::size_t idx : {0, 1, 2, 8, 9, 10, 11, 12})
      nested_changed[idx] = 0x55;
    write_nested(nested_sample);
    auto original = slurp();
    write_nested(nested_changed);
    auto changed = slurp();
    auto before = create.template operator()<nested_record>(std::span<const char>{original});
    auto after = create.template operator()<nested_record>(std::span<const char>{changed});
    ASSERT_EQ(before.has_value(), after.has_value());
    ASSERT_TRUE(before.has_value());
    EXPECT_EQ((*before)["mid"_f]["hdr"_f]["kind"_f], (*after)["mid"_f]["hdr"_f]["kind"_f]);
    EXPECT_EQ((*before)["mid"_f]["hdr"_f]["len"_f], (*after)["mid"_f]["hdr"_f]["len"_f]);
    EXPECT_NE((*before)["tail"_f], (*after)["tail"_f]);
    EXPECT_NE((*before)["mid"_f]["hdr"_f]["narrow"_f], (*after)["mid"_f]["hdr"_f]["narrow"_f]);

    auto table_changed = table_sample;
    for(std::size_t idx : {0, 6, 7, 13, 14, 20, 21, 22, 23})
      table_changed[idx] = 0x55;
    write_bytes(table_sample);
    auto table_original = slurp();
    write_bytes(table_changed);
    auto table_other = slurp();
    auto t_before = create.template operator()<table_record>(std::span<const char>{table_original});
    auto t_after = create.template operator()<table_record>(std::span<const char>{table_other});
    ASSERT_EQ(t_before.has_value(), t_after.has_value());
    ASSERT_TRUE(t_before.has_value());
    for(std::size_t idx = 0; idx < 3; ++idx) {
      EXPECT_EQ((*t_before)["entries"_f][idx]["len"_f], (*t_after)["entries"_f][idx]["len"_f]);
      EXPECT_EQ((*t_before)["entries"_f][idx]["kind"_f], (*t_after)["entries"_f][idx]["kind"_f]);
      EXPECT_NE((*t_before)["entries"_f][idx]["flags"_f], (*t_after)["entries"_f][idx]["flags"_f]);
    }
  };
  expect_untouched([]<typename S>(std::span<const char> s) { return s2s::struct_view_le<S>(s); });
  expect_untouched([]<typename S>(std::span<const char> s) { return s2s::struct_view_be<S>(s); });
}
