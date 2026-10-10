#include <cstdint>
#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// The run-time half of delimited_write_ct.cpp. 065's happy path plus 066's
// rejection: a value that contains the delimiter, anywhere including its
// last byte, fails rather than writing something that would read back short.

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

using bounded_vector_then_tail =
  s2s::struct_field_list<
    s2s::vec_field<"data", u8, s2s::until<u8{0}>, s2s::max_bytes<79>>,
    s2s::basic_field<"tail", u16, 2_B>
  >;

// 067: a discovered length (until<>) feeding a computed one
// (size_from_fields) — PNG's tEXt chunk.
constexpr auto remaining = [](auto length, const std::string& keyword) -> std::size_t {
  return length - keyword.size() - 1;
};

using png_text_chunk =
  s2s::struct_field_list<
    s2s::basic_field<"length", u32, 4_B>,
    s2s::str_field<"keyword", s2s::until<u8{0}>, s2s::max_bytes<79>>,
    s2s::str_field<"text", s2s::size_from_fields<remaining, "length", "keyword">>
  >;
} /* namespace */

TEST(DelimitedWrite, EmitsTheValueFollowedByTheDelimiter) {
  using test_field_list = bounded_keyword_then_tail;

  test_field_list obj{};
  obj["keyword"_f] = "foo";
  obj["tail"_f] = u16{0x1122};

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::stream_cast_le<test_field_list>(stream, obj).has_value());
  EXPECT_EQ(stream.str(), std::string("foo\x00\x22\x11", 6));
}

TEST(DelimitedWrite, EmptyValueEmitsTheLoneDelimiterAndReadsBackEmpty) {
  using test_field_list = bounded_keyword_then_tail;

  test_field_list obj{};
  obj["keyword"_f] = "";
  obj["tail"_f] = u16{0x1122};

  FIELD_LIST_LE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE((*result)["keyword"_f].empty());
    EXPECT_EQ((*result)["tail"_f], u16{0x1122});
  });
}

TEST(DelimitedWrite, RoundTripsANulTerminatedKeywordAndAKnownField) {
  using test_field_list = bounded_keyword_then_tail;

  test_field_list obj{};
  obj["keyword"_f] = "foo";
  obj["tail"_f] = u16{0x1122};

  FIELD_LIST_LE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["keyword"_f], std::string("foo"));
    EXPECT_EQ((*result)["tail"_f], u16{0x1122});
  });
}

TEST(DelimitedWrite, RoundTripsAValueExactlyAtItsDeclaredBound) {
  using test_field_list = bounded_keyword_then_tail;

  test_field_list obj{};
  obj["keyword"_f] = std::string(79, 'a');
  obj["tail"_f] = u16{0x1122};

  FIELD_LIST_LE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["keyword"_f].size(), 79u);
    EXPECT_EQ((*result)["keyword"_f], obj["keyword"_f]);
    EXPECT_EQ((*result)["tail"_f], u16{0x1122});
  });
}

TEST(DelimitedWrite, RoundTripsAnUndeclaredBoundKeyword) {
  using test_field_list = undeclared_bound_keyword;

  test_field_list obj{};
  obj["keyword"_f] = "blob 123";

  FIELD_LIST_LE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["keyword"_f], std::string("blob 123"));
  });
}

TEST(DelimitedWrite, VecFieldEmitsTheValueFollowedByTheDelimiter) {
  using test_field_list = bounded_vector_then_tail;

  test_field_list obj{};
  obj["data"_f] = std::vector<u8>{'f', 'o', 'o'};
  obj["tail"_f] = u16{0x1122};

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::stream_cast_le<test_field_list>(stream, obj).has_value());
  EXPECT_EQ(stream.str(), std::string("foo\x00\x22\x11", 6));
}

TEST(DelimitedWrite, VecFieldEmptyValueEmitsTheLoneDelimiterAndReadsBackEmpty) {
  using test_field_list = bounded_vector_then_tail;

  test_field_list obj{};
  obj["data"_f] = std::vector<u8>{};
  obj["tail"_f] = u16{0x1122};

  FIELD_LIST_LE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE((*result)["data"_f].empty());
    EXPECT_EQ((*result)["tail"_f], u16{0x1122});
  });
}

TEST(DelimitedWrite, VecFieldRoundTripsUntilTheDelimiter) {
  using test_field_list = bounded_vector_then_tail;

  test_field_list obj{};
  obj["data"_f] = std::vector<u8>{'f', 'o', 'o'};
  obj["tail"_f] = u16{0x1122};

  FIELD_LIST_LE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["data"_f], obj["data"_f]);
    EXPECT_EQ((*result)["tail"_f], u16{0x1122});
  });
}

TEST(DelimitedWrite, VecFieldRoundTripsAValueExactlyAtItsDeclaredBound) {
  using test_field_list = bounded_vector_then_tail;

  test_field_list obj{};
  obj["data"_f] = std::vector<u8>(79, 'a');
  obj["tail"_f] = u16{0x1122};

  FIELD_LIST_LE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["data"_f].size(), 79u);
    EXPECT_EQ((*result)["data"_f], obj["data"_f]);
    EXPECT_EQ((*result)["tail"_f], u16{0x1122});
  });
}

TEST(DelimitedWrite, RejectsAValueContainingTheDelimiter) {
  using test_field_list = bounded_keyword_then_tail;

  test_field_list obj{};
  obj["keyword"_f] = std::string("fo\x00o", 4);
  obj["tail"_f] = u16{0x1122};

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto result = s2s::stream_cast_le<test_field_list>(stream, obj);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().failure_reason, s2s::error_reason::found_delimiter_in_value);
  EXPECT_EQ(result.error().failed_at, "keyword");
}

TEST(DelimitedWrite, RejectsAValueWhoseLastByteIsTheDelimiter) {
  using test_field_list = bounded_keyword_then_tail;

  test_field_list obj{};
  obj["keyword"_f] = std::string("foo\x00", 4);
  obj["tail"_f] = u16{0x1122};

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto result = s2s::stream_cast_le<test_field_list>(stream, obj);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().failure_reason, s2s::error_reason::found_delimiter_in_value);
  EXPECT_EQ(result.error().failed_at, "keyword");
}

TEST(DelimitedWrite, VecFieldRejectsAValueContainingTheDelimiter) {
  using test_field_list = bounded_vector_then_tail;

  test_field_list obj{};
  obj["data"_f] = std::vector<u8>{'f', 'o', 0x00, 'o'};
  obj["tail"_f] = u16{0x1122};

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto result = s2s::stream_cast_le<test_field_list>(stream, obj);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().failure_reason, s2s::error_reason::found_delimiter_in_value);
  EXPECT_EQ(result.error().failed_at, "data");
}

TEST(DelimitedWrite, VecFieldRejectsAValueWhoseLastByteIsTheDelimiter) {
  using test_field_list = bounded_vector_then_tail;

  test_field_list obj{};
  obj["data"_f] = std::vector<u8>{'f', 'o', 'o', 0x00};
  obj["tail"_f] = u16{0x1122};

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto result = s2s::stream_cast_le<test_field_list>(stream, obj);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().failure_reason, s2s::error_reason::found_delimiter_in_value);
  EXPECT_EQ(result.error().failed_at, "data");
}

// 067: the PNG tEXt shape round trips byte-identically, exercising 065's
// delimiter emission against a computed neighbour, with the keyword at PNG's
// stated 79-byte maximum.
TEST(DelimitedWrite, PngTextChunkRoundTripsWithA79ByteKeyword) {
  using test_field_list = png_text_chunk;

  const std::string keyword(79, 'k');
  const std::string text = "hello";
  const u32 length = static_cast<u32>(keyword.size() + 1 + text.size());

  test_field_list obj{};
  obj["length"_f] = length;
  obj["keyword"_f] = keyword;
  obj["text"_f] = text;

  std::string expected(reinterpret_cast<const char*>(&length), sizeof(length));
  expected += keyword;
  expected.push_back('\0');
  expected += text;

  FIELD_LIST_LE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    EXPECT_EQ(stream.str(), expected);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["keyword"_f], keyword);
    EXPECT_EQ((*result)["text"_f], text);
  });
}

TEST(DelimitedWrite, RoundTripsBigEndianTooEvenThoughDelimitedFieldsIgnoreOrder) {
  using test_field_list = bounded_keyword_then_tail;

  test_field_list obj{};
  obj["keyword"_f] = "foo";
  obj["tail"_f] = u16{0x1122};

  FIELD_LIST_BE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["keyword"_f], std::string("foo"));
    EXPECT_EQ((*result)["tail"_f], u16{0x1122});
  });
}
