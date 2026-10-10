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

// 063: vec_field<u8> is the same read loop as str_field, instantiated a
// second time — the same five cases, against std::vector<u8> instead of
// std::string.
using bounded_vector_then_tail =
  s2s::struct_field_list<
    s2s::vec_field<"data", u8, s2s::until<u8{0}>, s2s::max_bytes<79>>,
    s2s::basic_field<"tail", u16, 2_B>
  >;

// 067: a discovered length (until<>) feeding a computed one
// (size_from_fields) — PNG's tEXt chunk. The text is explicitly not
// terminated; its length is the chunk length minus the keyword's own
// discovered length minus the one delimiter byte.
constexpr auto remaining = [](auto length, const std::string& keyword) -> std::size_t {
  return length - keyword.size() - 1;
};

using png_text_chunk =
  s2s::struct_field_list<
    s2s::basic_field<"length", u32, 4_B>,
    s2s::str_field<"keyword", s2s::until<u8{0}>, s2s::max_bytes<79>>,
    s2s::str_field<"text", s2s::size_from_fields<remaining, "length", "keyword">>
  >;

// PREPARE_INPUT_FILE's argument is a macro parameter, not a variadic one: a
// brace-init list's top-level commas would be read as separate arguments.
// Bytes therefore live in a named array declared ahead of the macro call, and
// the macro body is a single file.write statement.
constexpr u8 keyword_then_tail_bytes[] = {'f', 'o', 'o', 0x00, 0x22, 0x11};
constexpr u8 empty_keyword_bytes[] = {0x00, 0x22, 0x11};
constexpr u8 short_stream_bytes[] = {'a', 'b', 'c'};
constexpr u8 high_delim_bytes[] = {'a', 'b', 0x80, 0x11};

// length = 11 (LE), "foo\0", "bar baz" — text runs to the chunk's declared
// end rather than carrying its own delimiter.
constexpr u8 png_text_chunk_bytes[] = {
  0x0b, 0x00, 0x00, 0x00,
  'f', 'o', 'o', 0x00,
  'b', 'a', 'r', ' ', 'b', 'a', 'z'
};
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

TEST(DelimitedRead, VecFieldReadsUntilTheDelimiterAndDropsIt) {
  PREPARE_INPUT_FILE({
    file.write(reinterpret_cast<const char*>(keyword_then_tail_bytes), sizeof(keyword_then_tail_bytes));
  });

  FIELD_LIST_SCHEMA = bounded_vector_then_tail;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    EXPECT_EQ(fields["data"_f], (std::vector<u8>{'f', 'o', 'o'}));
    EXPECT_EQ(fields["tail"_f], u16{0x1122});
  });
}

TEST(DelimitedRead, VecFieldDelimiterInFirstPositionYieldsAnEmptyValue) {
  PREPARE_INPUT_FILE({
    file.write(reinterpret_cast<const char*>(empty_keyword_bytes), sizeof(empty_keyword_bytes));
  });

  FIELD_LIST_SCHEMA = bounded_vector_then_tail;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    EXPECT_TRUE(fields["data"_f].empty());
    EXPECT_EQ(fields["tail"_f], u16{0x1122});
  });
}

TEST(DelimitedRead, VecFieldMaxBytesAcceptsAValueExactlyAtItsDeclaredBound) {
  FIELD_LIST_SCHEMA = bounded_vector_then_tail;

  PREPARE_INPUT_FILE({
    std::string value(79, 'a');
    value.push_back(static_cast<char>(0x00));
    value.push_back(static_cast<char>(0x22));
    value.push_back(static_cast<char>(0x11));
    file.write(value.data(), static_cast<std::streamsize>(value.size()));
  });

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["data"_f].size(), 79u);
  });
}

TEST(DelimitedRead, VecFieldMaxBytesRejectsAValueOneByteOverItsDeclaredBound) {
  using bounded_only =
    s2s::struct_field_list<
      s2s::vec_field<"data", u8, s2s::until<u8{0}>, s2s::max_bytes<79>>
    >;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  const std::string value(80, 'a');
  stream.write(value.data(), static_cast<std::streamsize>(value.size()));

  auto result = s2s::struct_cast_le<bounded_only>(stream);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().failure_reason, s2s::error_reason::delimiter_not_found);
  EXPECT_EQ(result.error().failed_at, "data");
}

TEST(DelimitedRead, VecFieldStreamRunningDryBeforeTheDelimiterReportsBufferExhaustion) {
  PREPARE_INPUT_FILE({
    file.write(reinterpret_cast<const char*>(short_stream_bytes), sizeof(short_stream_bytes));
  });

  FIELD_LIST_SCHEMA = bounded_vector_then_tail;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().failure_reason, s2s::error_reason::buffer_exhaustion);
    EXPECT_EQ(result.error().failed_at, "data");
  });
}

TEST(DelimitedRead, VecFieldRejectsAnUnterminatedValueAtTheDefaultBoundWithNothingDeclared) {
  using undeclared_bound_vector =
    s2s::struct_field_list<
      s2s::vec_field<"data", u8, s2s::until<u8{0}>>
    >;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  const std::string value(s2s::default_max_bytes + 1, 'a');
  stream.write(value.data(), static_cast<std::streamsize>(value.size()));

  auto result = s2s::struct_cast_le<undeclared_bound_vector>(stream);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().failure_reason, s2s::error_reason::delimiter_not_found);
  EXPECT_EQ(result.error().failed_at, "data");
}

TEST(DelimitedRead, PngTextChunkComposesADiscoveredLengthWithAComputedOne) {
  PREPARE_INPUT_FILE({
    file.write(reinterpret_cast<const char*>(png_text_chunk_bytes), sizeof(png_text_chunk_bytes));
  });

  FIELD_LIST_SCHEMA = png_text_chunk;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    EXPECT_EQ(std::string_view{fields["keyword"_f]}, "foo");
    EXPECT_EQ(std::string_view{fields["text"_f]}, "bar baz");
  });
}

TEST(DelimitedRead, PngTextChunkAcceptsA79ByteKeywordAtItsStatedMaximum) {
  const std::string keyword(79, 'k');
  const std::string text = "hello";
  const u32 length = static_cast<u32>(keyword.size() + 1 + text.size());

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  stream.write(reinterpret_cast<const char*>(&length), sizeof(length));
  stream.write(keyword.data(), static_cast<std::streamsize>(keyword.size()));
  stream.put('\0');
  stream.write(text.data(), static_cast<std::streamsize>(text.size()));

  auto result = s2s::struct_cast_le<png_text_chunk>(stream);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ((*result)["keyword"_f].size(), 79u);
  EXPECT_EQ((*result)["keyword"_f], keyword);
  EXPECT_EQ((*result)["text"_f], text);
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
