#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// The run-time half of sentinel_records_write_ct.cpp, against a real ofstream.
// 071 only: a vector holding a sentinel-matching element is 072's.

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
