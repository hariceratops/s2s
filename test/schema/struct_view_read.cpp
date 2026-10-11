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
