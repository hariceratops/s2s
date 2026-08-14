#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

namespace {
auto area_of = [](auto rows, auto cols) { return rows * cols; };

using computed_schema =
  s2s::struct_field_list<
    s2s::basic_field<"rows", u32, 4_B>,
    s2s::basic_field<"cols", u32, 4_B>,
    s2s::vec_field<
      "cells",
      u16,
      s2s::len_from_fields<area_of, "rows", "cols">
    >
  >;

// Two variable-sized fields sharing one length field.
using fanout_schema =
  s2s::struct_field_list<
    s2s::basic_field<"len", u32, 4_B>,
    s2s::vec_field<"a", u16, s2s::len_from_field<"len">>,
    s2s::vec_field<"b", u32, s2s::len_from_field<"len">>
  >;
} /* namespace */

TEST(SizeAxisWrite, RoundTripsWhenSourcesAgreeWithTheContainer) {
  using test_field_list = computed_schema;

  computed_schema obj{};
  obj["rows"_f] = 2;
  obj["cols"_f] = 3;
  obj["cells"_f] = std::vector<u16>{1, 2, 3, 4, 5, 6};

  FIELD_LIST_LE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["rows"_f], 2u);
    EXPECT_EQ((*result)["cols"_f], 3u);
    EXPECT_EQ((*result)["cells"_f], (std::vector<u16>{1, 2, 3, 4, 5, 6}));
  });
}

TEST(SizeAxisWrite, RoundTripsBigEndian) {
  using test_field_list = computed_schema;

  computed_schema obj{};
  obj["rows"_f] = 2;
  obj["cols"_f] = 2;
  obj["cells"_f] = std::vector<u16>{0x1122, 0x3344, 0x5566, 0x7788};

  FIELD_LIST_BE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["cells"_f], (std::vector<u16>{0x1122, 0x3344, 0x5566, 0x7788}));
  });
}

// The callable has no inverse, so a disagreement cannot be repaired by
// rewriting either side — emitting either value would produce a stream that
// does not read back as what was handed in.
TEST(SizeAxisWrite, RejectsSourcesThatDisagreeWithTheContainer) {
  using test_field_list = computed_schema;

  computed_schema obj{};
  obj["rows"_f] = 2;
  obj["cols"_f] = 3;
  obj["cells"_f] = std::vector<u16>{1, 2, 3, 4};

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::stream_cast_le<test_field_list>(stream, obj);
  ASSERT_FALSE(written.has_value());
  EXPECT_EQ(written.error().failure_reason, s2s::error_reason::found_contradicting_length);
  EXPECT_EQ(written.error().failed_at, "cells");
  // rows and cols are already out, but nothing of cells is.
  EXPECT_EQ(stream.str().size(), 8u);
}

// Issue 005 must not capture these: an arbitrary callable cannot be inverted,
// so the fields feeding it are ordinary writable data.
TEST(SizeAxisWrite, SourcesFeedingTheCallableStayAssignable) {
  computed_schema obj{};
  obj["rows"_f] = 7;
  obj["cols"_f] = 9;
  EXPECT_EQ(obj["rows"_f], 7u);
  EXPECT_EQ(obj["cols"_f], 9u);

  static_assert(
    !s2s::is_derived_field<meta::type_id<s2s::field_list_metadata<
      s2s::basic_field<"rows", u32, 4_B>,
      s2s::basic_field<"cols", u32, 4_B>,
      s2s::vec_field<
        "cells", u16,
        s2s::len_from_fields<area_of, "rows", "cols">
      >
    >>>("rows"));
}

TEST(SizeAxisWrite, RoundTripsFanOutWhenDependentsAgree) {
  using test_field_list = fanout_schema;

  fanout_schema obj{};
  obj["a"_f] = std::vector<u16>{0x1111, 0x2222, 0x3333};
  obj["b"_f] = std::vector<u32>{0xaaaaaaaa, 0xbbbbbbbb, 0xcccccccc};

  FIELD_LIST_LE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["a"_f].size(), 3u);
    EXPECT_EQ((*result)["b"_f], (std::vector<u32>{0xaaaaaaaa, 0xbbbbbbbb, 0xcccccccc}));
  });
}

// One length field cannot describe two containers of different sizes. Since
// the length is written first, this has to fail there, before any bytes.
TEST(SizeAxisWrite, RejectsFanOutContradiction) {
  using test_field_list = fanout_schema;

  fanout_schema obj{};
  obj["a"_f] = std::vector<u16>{0x1111, 0x2222, 0x3333};
  obj["b"_f] = std::vector<u32>{0xaaaaaaaa};

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::stream_cast_le<test_field_list>(stream, obj);
  ASSERT_FALSE(written.has_value());
  EXPECT_EQ(written.error().failure_reason, s2s::error_reason::found_contradicting_length);
  EXPECT_EQ(written.error().failed_at, "len");
  EXPECT_TRUE(stream.str().empty());
}

// 004's width check is on the derived value, so sharing a length field does
// not let an over-wide length slip through.
TEST(SizeAxisWrite, AppliesTheWidthCheckToAFannedOutLength) {
  using test_field_list =
    s2s::struct_field_list<
      s2s::basic_field<"len", u32, 1_B>,
      s2s::vec_field<"a", u8, s2s::len_from_field<"len">>,
      s2s::vec_field<"b", u8, s2s::len_from_field<"len">>
    >;

  test_field_list obj{};
  obj["a"_f] = std::vector<u8>(300, 0x11);
  obj["b"_f] = std::vector<u8>(300, 0x22);

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::stream_cast_le<test_field_list>(stream, obj);
  ASSERT_FALSE(written.has_value());
  EXPECT_EQ(written.error().failure_reason, s2s::error_reason::validation_failure);
  EXPECT_EQ(written.error().failed_at, "len");
}

// TODO(045): the run-time half — a defaulted size and a 2_B size must emit the
// same bytes as the 2_B spelling they replace. Comparing the
// emitted buffer against the pre-migration bytes is the evidence that the
// migration was a spelling change and nothing else.
TEST(SizeAxisWrite, WritesAFieldWhoseSizeIsDefaultedToSizeofT) {
  using test_field_list =
    s2s::struct_field_list<
      s2s::basic_field<"a", u16>,
      s2s::basic_field<"b", u32, 2_B>
    >;

  test_field_list obj{};
  obj["a"_f] = u16{0x1122};
  obj["b"_f] = 0x3344u;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::stream_cast_le<test_field_list>(stream, obj).has_value());

  // An omitted size is sizeof(T); a narrower one truncates to what it declares.
  EXPECT_EQ(stream.str().size(), 4u);
  EXPECT_EQ(static_cast<u8>(stream.str()[0]), 0x22);
  EXPECT_EQ(static_cast<u8>(stream.str()[2]), 0x44);
}
