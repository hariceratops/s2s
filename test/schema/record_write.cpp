#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

namespace {
using point =
  s2s::struct_field_list<
    s2s::basic_field<"x", u32, 4_B>,
    s2s::basic_field<"y", u32, 4_B>
  >;

using record_schema =
  s2s::struct_field_list<
    s2s::struct_field<"origin", point>,
    s2s::array_of_records<"corners", point, 2>,
    s2s::basic_field<"count", u32, 4_B>,
    s2s::vector_of_records<"path", point, s2s::len_from_field<"count">>
  >;

auto make_point(u32 x, u32 y) -> point {
  point p{};
  p["x"_f] = x;
  p["y"_f] = y;
  return p;
}

auto populated() -> record_schema {
  record_schema obj{};
  obj["origin"_f] = make_point(0x1111, 0x2222);
  obj["corners"_f][0] = make_point(0x3333, 0x4444);
  obj["corners"_f][1] = make_point(0x5555, 0x6666);
  obj["path"_f] = std::vector<point>{make_point(0x7777, 0x8888), make_point(0x9999, 0xaaaa)};
  return obj;
}

auto expect_matches_populated(const record_schema& actual) -> void {
  EXPECT_EQ(actual["origin"_f]["x"_f], 0x1111u);
  EXPECT_EQ(actual["origin"_f]["y"_f], 0x2222u);
  EXPECT_EQ(actual["corners"_f][0]["x"_f], 0x3333u);
  EXPECT_EQ(actual["corners"_f][1]["y"_f], 0x6666u);
  ASSERT_EQ(actual["path"_f].size(), 2u);
  EXPECT_EQ(actual["path"_f][0]["x"_f], 0x7777u);
  EXPECT_EQ(actual["path"_f][1]["y"_f], 0xaaaau);
}
} /* namespace */

TEST(RecordWrite, RoundTripsRecordFieldsLittleEndian) {
  using test_field_list = record_schema;

  FIELD_LIST_LE_ROUNDTRIP_CHECK(populated(), {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    expect_matches_populated(*result);
  });
}

TEST(RecordWrite, RoundTripsRecordFieldsBigEndian) {
  using test_field_list = record_schema;

  FIELD_LIST_BE_ROUNDTRIP_CHECK(populated(), {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    expect_matches_populated(*result);
  });
}

// Recursion must not reorder anything: the bytes are the concatenation of
// every leaf in declaration order, at every depth.
TEST(RecordWrite, PreservesDeclarationOrderAtEveryNestingLevel) {
  using test_field_list =
    s2s::struct_field_list<
      s2s::basic_field<"lead", u16, 2_B>,
      s2s::struct_field<
        "inner",
        s2s::struct_field_list<
          s2s::basic_field<"a", u16, 2_B>,
          s2s::basic_field<"b", u16, 2_B>
        >
      >,
      s2s::basic_field<"trail", u16, 2_B>
    >;

  test_field_list obj{};
  obj["lead"_f] = 0x1111;
  obj["inner"_f]["a"_f] = 0x2222;
  obj["inner"_f]["b"_f] = 0x3333;
  obj["trail"_f] = 0x4444;

  std::stringstream be(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::stream_cast_be<test_field_list>(be, obj).has_value());
  EXPECT_EQ(be.str(), std::string("\x11\x11\x22\x22\x33\x33\x44\x44", 8));
}

TEST(RecordWrite, RunsConstraintCheckersInsideNestedRecords) {
  // Triggered by an ordinary constraint rather than a magic field: a field
  // pinned to eq takes its value from the constraint, so it can no longer
  // carry a wrong one.
  using tagged =
    s2s::struct_field_list<
      s2s::basic_field<"tag", u32, 4_B, s2s::lt{100u}>,
      s2s::basic_field<"payload", u32, 4_B>
    >;
  using test_field_list =
    s2s::struct_field_list<
      s2s::basic_field<"lead", u32, 4_B>,
      s2s::struct_field<"inner", tagged>
    >;

  test_field_list obj{};
  obj["lead"_f] = 0x11223344;
  obj["inner"_f]["tag"_f] = 0xbeefbeef;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::stream_cast_le<test_field_list>(stream, obj);
  ASSERT_FALSE(written.has_value());
  EXPECT_EQ(written.error().failure_reason, s2s::error_reason::validation_failure);
  // Names the outer record field, not "tag" — rw_result carries no name, so
  // the outermost fold is the only place one is attached. Matches read.
  EXPECT_EQ(written.error().failed_at, "inner");
  EXPECT_EQ(stream.str().size(), 4u);
}

TEST(RecordWrite, RunsConstraintCheckersInsideVectorOfRecords) {
  using tagged =
    s2s::struct_field_list<
      s2s::basic_field<"tag", u32, 4_B, s2s::lt{100u}>
    >;
  using test_field_list =
    s2s::struct_field_list<
      s2s::basic_field<"count", u32, 4_B>,
      s2s::vector_of_records<"items", tagged, s2s::len_from_field<"count">>
    >;

  tagged good{};
  good["tag"_f] = 42u;
  tagged bad{};
  bad["tag"_f] = 0xdeadbeef;

  test_field_list obj{};
  obj["items"_f] = std::vector<tagged>{good, bad};

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::stream_cast_le<test_field_list>(stream, obj);
  ASSERT_FALSE(written.has_value());
  EXPECT_EQ(written.error().failed_at, "items");
  // The count and the first, valid record are out; the second contributed
  // nothing.
  EXPECT_EQ(stream.str().size(), 8u);
}

// A length field inside a record element describes that element's own data,
// not the outer struct's.
TEST(RecordWrite, DerivesNestedLengthsFromTheirOwnElement) {
  using sized_record =
    s2s::struct_field_list<
      s2s::basic_field<"len", u32, 4_B>,
      s2s::vec_field<"data", u8, s2s::len_from_field<"len">>
    >;
  using test_field_list =
    s2s::struct_field_list<
      s2s::basic_field<"count", u32, 4_B>,
      s2s::vector_of_records<"records", sized_record, s2s::len_from_field<"count">>
    >;

  sized_record short_one{};
  short_one["data"_f] = std::vector<u8>{0xaa};
  sized_record long_one{};
  long_one["data"_f] = std::vector<u8>{0xbb, 0xcc, 0xdd};

  test_field_list obj{};
  obj["records"_f] = std::vector<sized_record>{short_one, long_one};

  FIELD_LIST_BE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ((*result)["records"_f].size(), 2u);
    EXPECT_EQ((*result)["records"_f][0]["data"_f], (std::vector<u8>{0xaa}));
    EXPECT_EQ((*result)["records"_f][1]["data"_f], (std::vector<u8>{0xbb, 0xcc, 0xdd}));
  });
}

// The freeze reaches a nested record's own fields: write_nested runs
// stream_cast_impl on the inner list, where the magic field is a member of
// that list and so is frozen there in its own right.
TEST(RecordWrite, WritesAFrozenFieldInsideANestedRecord) {
  using tagged =
    s2s::struct_field_list<
      s2s::magic_number<"tag", u32, 4_B, 0xdeadbeef>,
      s2s::basic_field<"payload", u32, 4_B>
    >;
  using test_field_list =
    s2s::struct_field_list<
      s2s::basic_field<"lead", u32, 4_B>,
      s2s::struct_field<"inner", tagged>
    >;

  test_field_list obj{};
  obj["lead"_f] = 0x11223344;
  obj["inner"_f]["payload"_f] = 0x55667788;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::stream_cast_be<test_field_list>(stream, obj).has_value());
  EXPECT_EQ(stream.str(),
            std::string("\x11\x22\x33\x44"
                        "\xde\xad\xbe\xef"
                        "\x55\x66\x77\x88", 12));
}
