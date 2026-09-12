#include <cstdint>
#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// The run-time half of delimited_read_ct.cpp. The compile-time cases cover
// the matrix at a small declared max_bytes; this file additionally exercises
// the delimiter_not_found path at the *default* 16 MiB bound, which the
// compile-time tier cannot afford (dev/design's F6).

namespace {
using bounded_keyword_then_tail =
  s2s::struct_field_list<
    s2s::str_field<"keyword", s2s::until<u8{0}>, s2s::max_bytes<79>>,
    s2s::basic_field<"tail", u16, 2_B>
  >;

using undeclared_bound_keyword =
  s2s::struct_field_list<
    s2s::str_field<"keyword", s2s::until<u8{0}>>
  >;

// PREPARE_INPUT_FILE's argument is a macro parameter, not a variadic one: a
// brace-init list's top-level commas would be read as separate arguments.
// Bytes therefore live in a named array declared ahead of the macro call, and
// the macro body is a single file.write statement.
constexpr u8 keyword_then_tail_bytes[] = {'f', 'o', 'o', 0x00, 0x22, 0x11};
constexpr u8 empty_keyword_bytes[] = {0x00, 0x22, 0x11};
constexpr u8 short_stream_bytes[] = {'a', 'b', 'c'};
constexpr u8 high_delim_bytes[] = {'a', 'b', 0x80, 0x11};
} /* namespace */

TEST(DelimitedRead, ReadsANulTerminatedKeywordAndDropsTheDelimiter) {
  PREPARE_INPUT_FILE({
    file.write(reinterpret_cast<const char*>(keyword_then_tail_bytes), sizeof(keyword_then_tail_bytes));
  });

  FIELD_LIST_SCHEMA = bounded_keyword_then_tail;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    EXPECT_EQ(fields["keyword"_f].size(), 3u);
    EXPECT_EQ(std::string_view{fields["keyword"_f]}, "foo");
    EXPECT_EQ(fields["tail"_f], u16{0x1122});
  });
}

TEST(DelimitedRead, DelimiterInFirstPositionYieldsAnEmptyValue) {
  PREPARE_INPUT_FILE({
    file.write(reinterpret_cast<const char*>(empty_keyword_bytes), sizeof(empty_keyword_bytes));
  });

  FIELD_LIST_SCHEMA = bounded_keyword_then_tail;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    EXPECT_TRUE(fields["keyword"_f].empty());
    EXPECT_EQ(fields["tail"_f], u16{0x1122});
  });
}

TEST(DelimitedRead, MaxBytesAcceptsAValueExactlyAtItsDeclaredBound) {
  FIELD_LIST_SCHEMA = bounded_keyword_then_tail;

  PREPARE_INPUT_FILE({
    std::string value(79, 'a');
    value.push_back(static_cast<char>(0x00));
    value.push_back(static_cast<char>(0x22));
    value.push_back(static_cast<char>(0x11));
    file.write(value.data(), static_cast<std::streamsize>(value.size()));
  });

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["keyword"_f].size(), 79u);
  });
}

TEST(DelimitedRead, MaxBytesRejectsAValueOneByteOverItsDeclaredBound) {
  using bounded_only =
    s2s::struct_field_list<
      s2s::str_field<"keyword", s2s::until<u8{0}>, s2s::max_bytes<79>>
    >;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  const std::string value(80, 'a');
  stream.write(value.data(), static_cast<std::streamsize>(value.size()));

  auto result = s2s::struct_cast_le<bounded_only>(stream);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().failure_reason, s2s::error_reason::delimiter_not_found);
  EXPECT_EQ(result.error().failed_at, "keyword");
}

TEST(DelimitedRead, StreamRunningDryBeforeTheDelimiterReportsBufferExhaustion) {
  PREPARE_INPUT_FILE({
    file.write(reinterpret_cast<const char*>(short_stream_bytes), sizeof(short_stream_bytes));
  });

  FIELD_LIST_SCHEMA = bounded_keyword_then_tail;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().failure_reason, s2s::error_reason::buffer_exhaustion);
    EXPECT_EQ(result.error().failed_at, "keyword");
  });
}

// The headline behaviour of the third outcome: a field declaring no bound
// falls back to default_max_bytes, and a stream with no delimiter anywhere in
// 16 MiB is rejected as delimiter_not_found rather than read forever.
TEST(DelimitedRead, RejectsAnUnterminatedValueAtTheDefaultBoundWithNothingDeclared) {
  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  const std::string value(s2s::default_max_bytes + 1, 'a');
  stream.write(value.data(), static_cast<std::streamsize>(value.size()));

  auto result = s2s::struct_cast_le<undeclared_bound_keyword>(stream);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().failure_reason, s2s::error_reason::delimiter_not_found);
  EXPECT_EQ(result.error().failed_at, "keyword");
}

TEST(DelimitedRead, DelimiterAtOrAbove0x80IsMatchedNotSignExtendedPast) {
  using high_delim =
    s2s::struct_field_list<
      s2s::str_field<"keyword", s2s::until<u8{0x80}>, s2s::max_bytes<79>>
    >;

  PREPARE_INPUT_FILE({
    file.write(reinterpret_cast<const char*>(high_delim_bytes), sizeof(high_delim_bytes));
  });

  FIELD_LIST_SCHEMA = high_delim;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::string_view{(*result)["keyword"_f]}, "ab");
  });
}
