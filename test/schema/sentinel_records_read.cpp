#include <cstdint>
#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// The run-time half of sentinel_records_read_ct.cpp. Additionally exercises
// sentinel_not_found at the *default* 16 MiB bound, which the compile-time
// tier cannot afford.

namespace {
using sub_block =
  s2s::struct_field_list<
    s2s::basic_field<"size", u8, 1_B>,
    s2s::vec_field<"data", u8, s2s::len_from_field<"size">>
  >;

using image_data =
  s2s::struct_field_list<
    s2s::basic_field<"lzw_min_code_size", u8, 1_B>,
    s2s::vector_of_records<"blocks", sub_block, s2s::until_field_equals<"size", u8{0}>>,
    s2s::basic_field<"tail", u16, 2_B>
  >;

using label =
  s2s::struct_field_list<
    s2s::basic_field<"len", u8, 1_B>,
    s2s::str_field<"text", s2s::len_from_field<"len">>
  >;

using dns_name =
  s2s::struct_field_list<
    s2s::vector_of_records<"labels", label, s2s::until_field_equals<"len", u8{0}>>,
    s2s::basic_field<"tail", u16, 2_B>
  >;

using bounded_blocks =
  s2s::struct_field_list<
    s2s::vector_of_records<"blocks", sub_block,
      s2s::until_field_equals<"size", u8{0}>,
      s2s::max_bytes<3 * sizeof(sub_block)>>
  >;

using undeclared_bound_blocks =
  s2s::struct_field_list<
    s2s::vector_of_records<"blocks", sub_block, s2s::until_field_equals<"size", u8{0}>>
  >;

constexpr u8 gif_bytes[] = {0x08, 0x02, 'a', 'b', 0x01, 'c', 0x00, 0x22, 0x11};
constexpr u8 dns_bytes[] = {0x03, 'w', 'w', 'w', 0x01, 'a', 0x00, 0x22, 0x11};
constexpr u8 empty_gif_bytes[] = {0x08, 0x00, 0x22, 0x11};
constexpr u8 over_bound_bytes[] = {0x01, 'a', 0x01, 'b', 0x01, 'c', 0x01, 'd', 0x00};
constexpr u8 exact_bound_bytes[] = {0x01, 'a', 0x01, 'b', 0x01, 'c', 0x00};
constexpr u8 dry_between_bytes[] = {0x01, 'a', 0x01, 'b'};
constexpr u8 dry_mid_bytes[] = {0x01, 'a', 0x05};
constexpr char one_byte_block[] = {0x01, 'x'};

auto write_blocks_past_default_bound(std::ofstream& file) -> void {
  for(std::size_t i = 0; i <= s2s::default_max_bytes / sizeof(sub_block); ++i)
    file.write(one_byte_block, sizeof(one_byte_block));
}
} /* namespace */

TEST(SentinelRecordsRead, GifSubBlockRunDropsTheTerminatorAndReadsOn) {
  PREPARE_INPUT_FILE({
    file.write(reinterpret_cast<const char*>(gif_bytes), sizeof(gif_bytes));
  });

  FIELD_LIST_SCHEMA = image_data;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    EXPECT_EQ(fields["lzw_min_code_size"_f], u8{8});
    ASSERT_EQ(fields["blocks"_f].size(), 2u);
    EXPECT_EQ(fields["blocks"_f][0]["data"_f].size(), 2u);
    EXPECT_EQ(fields["blocks"_f][1]["data"_f][0], u8{'c'});
    EXPECT_EQ(fields["tail"_f], u16{0x1122});
  });
}

TEST(SentinelRecordsRead, DnsLabelListReadsUpToTheZeroLengthLabel) {
  PREPARE_INPUT_FILE({
    file.write(reinterpret_cast<const char*>(dns_bytes), sizeof(dns_bytes));
  });

  FIELD_LIST_SCHEMA = dns_name;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    ASSERT_EQ(fields["labels"_f].size(), 2u);
    EXPECT_EQ(fields["labels"_f][0]["text"_f], "www");
    EXPECT_EQ(fields["tail"_f], u16{0x1122});
  });
}

TEST(SentinelRecordsRead, TerminatorInFirstPositionYieldsAnEmptyVector) {
  PREPARE_INPUT_FILE({
    file.write(reinterpret_cast<const char*>(empty_gif_bytes), sizeof(empty_gif_bytes));
  });

  FIELD_LIST_SCHEMA = image_data;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE((*result)["blocks"_f].empty());
    EXPECT_EQ((*result)["tail"_f], u16{0x1122});
  });
}

TEST(SentinelRecordsRead, MaxBytesAcceptsExactlyTheBoundsWorthOfElements) {
  PREPARE_INPUT_FILE({
    file.write(reinterpret_cast<const char*>(exact_bound_bytes), sizeof(exact_bound_bytes));
  });

  FIELD_LIST_SCHEMA = bounded_blocks;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["blocks"_f].size(), 3u);
  });
}

TEST(SentinelRecordsRead, MaxBytesRejectsOneElementMoreWithSentinelNotFound) {
  PREPARE_INPUT_FILE({
    file.write(reinterpret_cast<const char*>(over_bound_bytes), sizeof(over_bound_bytes));
  });

  FIELD_LIST_SCHEMA = bounded_blocks;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().failure_reason, s2s::error_reason::sentinel_not_found);
    EXPECT_EQ(result.error().failed_at, "blocks");
  });
}

TEST(SentinelRecordsRead, RunningDryBetweenElementsIsBufferExhaustion) {
  PREPARE_INPUT_FILE({
    file.write(reinterpret_cast<const char*>(dry_between_bytes), sizeof(dry_between_bytes));
  });

  FIELD_LIST_SCHEMA = bounded_blocks;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().failure_reason, s2s::buffer_exhaustion);
  });
}

TEST(SentinelRecordsRead, RunningDryMidElementIsBufferExhaustion) {
  PREPARE_INPUT_FILE({
    file.write(reinterpret_cast<const char*>(dry_mid_bytes), sizeof(dry_mid_bytes));
  });

  FIELD_LIST_SCHEMA = bounded_blocks;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().failure_reason, s2s::buffer_exhaustion);
  });
}

TEST(SentinelRecordsRead, DefaultBoundStopsARunThatNeverYieldsItsSentinel) {
  // Past the 16 MiB default in-memory bound, with no sentinel anywhere.
  PREPARE_INPUT_FILE({
    write_blocks_past_default_bound(file);
  });

  FIELD_LIST_SCHEMA = undeclared_bound_blocks;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().failure_reason, s2s::error_reason::sentinel_not_found);
  });
}
