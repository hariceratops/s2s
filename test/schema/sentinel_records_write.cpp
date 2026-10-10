#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// The run-time half of sentinel_records_write_ct.cpp, against a real ofstream.

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

using tagged =
  s2s::struct_field_list<
    s2s::basic_field<"kind", u8, 1_B>,
    s2s::basic_field<"value", u16, 2_B>
  >;

using tagged_run =
  s2s::struct_field_list<
    s2s::vector_of_records<"items", tagged, s2s::until_field_equals<"kind", u8{0xff}>>
  >;

using signed_record =
  s2s::struct_field_list<
    s2s::basic_field<"v", signed char, 1_B>
  >;

using signed_run =
  s2s::struct_field_list<
    s2s::vector_of_records<"run", signed_record, s2s::until_field_equals<"v", 0xff>>
  >;

constexpr char path[] = "test_output.bin";

template <typename list>
auto write_to_file(const list& obj) -> bool {
  std::ofstream file(path, std::ios::out | std::ios::binary);
  return s2s::stream_cast_le<list>(file, obj).has_value();
}

auto file_bytes() -> std::string {
  std::ifstream file(path, std::ios::in | std::ios::binary);
  return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}
} /* namespace */

TEST(SentinelRecordsWrite, GifSubBlockRunRoundTripsByteIdentically) {
  const std::string wire("\x08\x02" "ab" "\x01" "c" "\x00\x22\x11", 9);
  {
    std::ofstream in(path, std::ios::out | std::ios::binary);
    in.write(wire.data(), wire.size());
  }
  std::ifstream file(path, std::ios::in | std::ios::binary);
  auto read = s2s::struct_cast_le<image_data>(file);
  ASSERT_TRUE(read.has_value());
  file.close();

  ASSERT_TRUE(write_to_file<image_data>(*read));
  EXPECT_EQ(file_bytes(), wire);

  std::ifstream again(path, std::ios::in | std::ios::binary);
  auto reread = s2s::struct_cast_le<image_data>(again);
  ASSERT_TRUE(reread.has_value());
  EXPECT_EQ((*reread)["blocks"_f].size(), 2u);
}

TEST(SentinelRecordsWrite, DnsLabelListRoundTripsByteIdentically) {
  const std::string wire("\x03" "www" "\x01" "a" "\x00\x22\x11", 9);
  {
    std::ofstream in(path, std::ios::out | std::ios::binary);
    in.write(wire.data(), wire.size());
  }
  std::ifstream file(path, std::ios::in | std::ios::binary);
  auto read = s2s::struct_cast_le<dns_name>(file);
  ASSERT_TRUE(read.has_value());
  file.close();

  ASSERT_TRUE(write_to_file<dns_name>(*read));
  EXPECT_EQ(file_bytes(), wire);
}

TEST(SentinelRecordsWrite, EmptyVectorEmitsOnlyTheTerminatorAndReadsBackEmpty) {
  image_data obj{};
  obj["lzw_min_code_size"_f] = u8{8};
  obj["tail"_f] = u16{0x1122};

  ASSERT_TRUE(write_to_file<image_data>(obj));
  EXPECT_EQ(file_bytes(), std::string("\x08\x00\x22\x11", 4));

  std::ifstream file(path, std::ios::in | std::ios::binary);
  auto read = s2s::struct_cast_le<image_data>(file);
  ASSERT_TRUE(read.has_value());
  EXPECT_TRUE((*read)["blocks"_f].empty());
}

TEST(SentinelRecordsWrite, BigEndianWriteEmitsTheSameTerminator) {
  image_data obj{};
  obj["lzw_min_code_size"_f] = u8{8};
  obj["tail"_f] = u16{0x1122};

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::stream_cast_be<image_data>(stream, obj).has_value());
  EXPECT_EQ(stream.str(), std::string("\x08\x00\x11\x22", 4));
}

TEST(SentinelRecordsWrite, HandBuiltNonEmptyElementsAreAcceptedAndTerminated) {
  image_data obj{};
  obj["lzw_min_code_size"_f] = u8{8};
  obj["blocks"_f].resize(2);
  obj["blocks"_f][0]["data"_f] = {'a', 'b'};
  obj["blocks"_f][1]["data"_f] = {'c'};
  obj["tail"_f] = u16{0x1122};

  ASSERT_TRUE(write_to_file<image_data>(obj));
  EXPECT_EQ(file_bytes(), std::string("\x08\x02" "ab" "\x01" "c" "\x00\x22\x11", 9));
}

TEST(SentinelRecordsWrite, EmptyDataElementIsRejectedAtAnyPositionBeforeItsBytes) {
  for(std::size_t pos = 0; pos < 3; ++pos) {
    image_data obj{};
    obj["lzw_min_code_size"_f] = u8{8};
    obj["blocks"_f].resize(3);
    for(std::size_t idx = 0; idx < 3; ++idx)
      if(idx != pos)
        obj["blocks"_f][idx]["data"_f] = {static_cast<u8>('a' + idx)};

    std::string expected("\x08", 1);
    for(std::size_t idx = 0; idx < pos; ++idx)
      expected += std::string("\x01", 1) + static_cast<char>('a' + idx);

    std::ofstream file(path, std::ios::out | std::ios::binary);
    auto result = s2s::stream_cast_le<image_data>(file, obj);
    file.close();

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().failure_reason, s2s::error_reason::found_sentinel_in_sequence);
    EXPECT_EQ(result.error().failed_at, "blocks");
    EXPECT_EQ(file_bytes(), expected);
  }
}

TEST(SentinelRecordsWrite, ReadElementWithClearedDataIsRejectedDespiteStaleStoredSize) {
  const std::string wire("\x08\x02" "ab" "\x01" "c" "\x00\x22\x11", 9);
  {
    std::ofstream in(path, std::ios::out | std::ios::binary);
    in.write(wire.data(), wire.size());
  }
  std::ifstream file(path, std::ios::in | std::ios::binary);
  auto read = s2s::struct_cast_le<image_data>(file);
  ASSERT_TRUE(read.has_value());
  (*read)["blocks"_f][0]["data"_f].clear();

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto result = s2s::stream_cast_le<image_data>(stream, *read);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().failure_reason, s2s::error_reason::found_sentinel_in_sequence);
}

TEST(SentinelRecordsWrite, OverflowingDerivedLengthIsAValidationFailureNotASentinelMatch) {
  image_data obj{};
  obj["blocks"_f].resize(1);
  obj["blocks"_f][0]["data"_f].resize(256);

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto result = s2s::stream_cast_le<image_data>(stream, obj);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().failure_reason, s2s::error_reason::validation_failure);
}

TEST(SentinelRecordsWrite, NonDerivedNamedFieldWritesRealElementsThenFullTerminator) {
  tagged_run obj{};
  obj["items"_f].resize(2);
  obj["items"_f][0]["kind"_f] = u8{1};
  obj["items"_f][0]["value"_f] = u16{0x1234};
  obj["items"_f][1]["kind"_f] = u8{2};
  obj["items"_f][1]["value"_f] = u16{0x5678};

  ASSERT_TRUE(write_to_file<tagged_run>(obj));
  EXPECT_EQ(file_bytes(), std::string("\x01\x34\x12\x02\x78\x56\xff\x00\x00", 9));
}

TEST(SentinelRecordsWrite, NonDerivedElementHoldingTheSentinelIsRejected) {
  for(std::size_t pos = 0; pos < 2; ++pos) {
    tagged_run obj{};
    obj["items"_f].resize(2);
    obj["items"_f][0]["kind"_f] = u8{1};
    obj["items"_f][1]["kind"_f] = u8{2};
    obj["items"_f][pos]["kind"_f] = u8{0xff};

    std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
    auto result = s2s::stream_cast_le<tagged_run>(stream, obj);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().failure_reason, s2s::error_reason::found_sentinel_in_sequence);
    EXPECT_EQ(result.error().failed_at, "items");
  }
}

TEST(SentinelRecordsWrite, SignedMinusOneIsTheSentinelOnWrite) {
  signed_run obj{};
  obj["run"_f].resize(1);
  obj["run"_f][0]["v"_f] = static_cast<signed char>(-1);

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto result = s2s::stream_cast_le<signed_run>(stream, obj);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().failure_reason, s2s::error_reason::found_sentinel_in_sequence);
}
