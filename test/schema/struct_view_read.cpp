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
