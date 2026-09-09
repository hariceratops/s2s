// The byte-order axis, read side — real-stream tier.
// Spec:   dev/specs/byte-order-is-data-not-a-template-parameter.md
// Design: dev/design/byte-order-is-data-not-a-template-parameter.md §10
//
// Routing is by constant-evaluability (dev/specs/compile-time-test-tier.md):
// anything holding a real stream is GoogleTest and lives here; the
// constant-evaluable mirror is byte_order_axis_read_ct.cpp.

#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"

using namespace s2s_literals;

namespace {
using order_marker =
  s2s::struct_field_list<
    s2s::fixed_array_field<"marker", u8, 2>
  >;

using tiff_header =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", order_marker,
      s2s::order_from<s2s::match_field<"marker">,
        s2s::order_switch<
          s2s::order_case<std::array<u8, 2>{'I', 'I'}, std::endian::little>,
          s2s::order_case<std::array<u8, 2>{'M', 'M'}, std::endian::big>>>>,
    s2s::magic_number<"magic", u16, 2_B, 42>,
    s2s::basic_field<"ifd_offset", u32>
  >;

// The same announcement in its ladder form, over the same marker.
constexpr auto marker_is_ii = [](std::array<u8, 2> m) { return m[0] == 'I' && m[1] == 'I'; };
constexpr auto marker_is_mm = [](std::array<u8, 2> m) { return m[0] == 'M' && m[1] == 'M'; };

using tiff_header_laddered =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", order_marker,
      s2s::order_from<
        s2s::order_if_else<
          s2s::order_branch<s2s::predicate<marker_is_ii, "marker">, std::endian::little>,
          s2s::order_branch<s2s::predicate<marker_is_mm, "marker">, std::endian::big>>>>,
    s2s::magic_number<"magic", u16, 2_B, 42>,
    s2s::basic_field<"ifd_offset", u32>
  >;

// A UTF-16 byte order mark, read as its two bytes rather than as one u16 —
// which it has to be, since a u16 marker would itself be order-dependent. The
// order follows from both fields together, which no single match_field can
// express.
using byte_order_mark =
  s2s::struct_field_list<
    s2s::basic_field<"first", u8, 1_B>,
    s2s::basic_field<"second", u8, 1_B>
  >;

constexpr auto bom_order = [](u8 first, u8 second) -> unsigned {
  if(first == 0xff && second == 0xfe) return 0u;
  if(first == 0xfe && second == 0xff) return 1u;
  return 2u;
};

using utf16_text =
  s2s::struct_field_list<
    s2s::announces_byte_order<"bom", byte_order_mark,
      s2s::order_from<s2s::compute<bom_order, unsigned, "first", "second">,
        s2s::order_switch<
          s2s::order_case<0u, std::endian::little>,
          s2s::order_case<1u, std::endian::big>>>>,
    s2s::fixed_array_field<"units", u16, 2>
  >;

// The over-reach guard's schema: everything after the announcement is
// order-dependent, and none of it is the check's business. If the walk reached
// past the announcing record this would not compile.
using tiff_with_order_dependent_siblings =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", order_marker,
      s2s::order_from<s2s::match_field<"marker">,
        s2s::order_switch<
          s2s::order_case<std::array<u8, 2>{'I', 'I'}, std::endian::little>,
          s2s::order_case<std::array<u8, 2>{'M', 'M'}, std::endian::big>>>>,
    s2s::magic_number<"magic", u16, 2_B, 42>,
    s2s::basic_field<"ifd_offset", u32>,
    s2s::fixed_array_field<"resolution", u16, 2>
  >;

// Nesting. The announcement sits inside a container, so the resolved order has
// to reach three places that are easy to get wrong independently: the
// container's own later fields, the fields after the container closes, and the
// same again one level further down.
using tiff_ifd =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", order_marker,
      s2s::order_from<s2s::match_field<"marker">,
        s2s::order_switch<
          s2s::order_case<std::array<u8, 2>{'I', 'I'}, std::endian::little>,
          s2s::order_case<std::array<u8, 2>{'M', 'M'}, std::endian::big>>>>,
    s2s::basic_field<"entry_count", u16, 2_B>
  >;

using nested_once =
  s2s::struct_field_list<
    s2s::struct_field<"ifd", tiff_ifd>,
    s2s::basic_field<"next_ifd", u32>
  >;

using nested_twice =
  s2s::struct_field_list<
    s2s::struct_field<"first", nested_once>,
    s2s::basic_field<"trailer", u32>
  >;

// Written a byte at a time rather than through an integer: the point of the
// test is which byte lands where, and reinterpreting an int would make the file
// depend on the host's own order.
template <std::size_t N>
auto write_bytes(std::ofstream& file, const std::array<u8, N>& bytes) -> void {
  file.write(reinterpret_cast<const char*>(bytes.data()), N);
}
} /* namespace */

// The whole claim of the feature: two different byte sequences, one schema, one
// entry point taking no order, and the same values out of both.
TEST(ByteOrderAxisRead, AnIIHeaderMakesTheRestOfTheFileLittleEndian) {
  PREPARE_INPUT_FILE({
    write_bytes(file, std::array<u8, 8>{'I', 'I', 0x2a, 0x00, 0x0d, 0xd0, 0xfe, 0xca});
  });

  FIELD_LIST_SCHEMA = tiff_header;

  FIELD_LIST_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    EXPECT_EQ(fields["magic"_f], 42);
    EXPECT_EQ(fields["ifd_offset"_f], 0xcafed00du);
  });
}

TEST(ByteOrderAxisRead, AnMMHeaderMakesTheRestOfTheFileBigEndian) {
  PREPARE_INPUT_FILE({
    write_bytes(file, std::array<u8, 8>{'M', 'M', 0x00, 0x2a, 0xca, 0xfe, 0xd0, 0x0d});
  });

  FIELD_LIST_SCHEMA = tiff_header;

  FIELD_LIST_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    EXPECT_EQ(fields["magic"_f], 42);
    EXPECT_EQ(fields["ifd_offset"_f], 0xcafed00du);
  });
}

// A marker matching no case is a file that is not this format — a magic
// mismatch, reported as one, against the announcing field's id.
TEST(ByteOrderAxisRead, AMarkerMatchingNoCaseFailsAsAValidationFailure) {
  PREPARE_INPUT_FILE({
    write_bytes(file, std::array<u8, 8>{'X', 'X', 0x2a, 0x00, 0x0d, 0xd0, 0xfe, 0xca});
  });

  FIELD_LIST_SCHEMA = tiff_header;

  FIELD_LIST_READ_CHECK({
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().failure_reason, s2s::error_reason::validation_failure);
    EXPECT_EQ(result.error().failed_at, "byte_order");
  });
}

TEST(ByteOrderAxisRead, ALadderResolvesTheOrder) {
  PREPARE_INPUT_FILE({
    write_bytes(file, std::array<u8, 8>{'M', 'M', 0x00, 0x2a, 0xca, 0xfe, 0xd0, 0x0d});
  });

  FIELD_LIST_SCHEMA = tiff_header_laddered;

  FIELD_LIST_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    EXPECT_EQ(fields["magic"_f], 42);
    EXPECT_EQ(fields["ifd_offset"_f], 0xcafed00du);
  });
}

// Two fields, one order: 0xff 0xfe and 0xfe 0xff are the same two bytes the
// other way round, so neither decides anything on its own.
TEST(ByteOrderAxisRead, ACallableResolvesTheOrderFromTwoFields) {
  PREPARE_INPUT_FILE({
    write_bytes(file, std::array<u8, 6>{0xff, 0xfe, 0x41, 0x00, 0x42, 0x00});
  });

  FIELD_LIST_SCHEMA = utf16_text;

  FIELD_LIST_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    EXPECT_EQ(fields["units"_f][0], 0x41);
    EXPECT_EQ(fields["units"_f][1], 0x42);
  });
}

TEST(ByteOrderAxisRead, TheSameTwoFieldsTheOtherWayRoundGiveTheOtherOrder) {
  PREPARE_INPUT_FILE({
    write_bytes(file, std::array<u8, 6>{0xfe, 0xff, 0x00, 0x41, 0x00, 0x42});
  });

  FIELD_LIST_SCHEMA = utf16_text;

  FIELD_LIST_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    EXPECT_EQ(fields["units"_f][0], 0x41);
    EXPECT_EQ(fields["units"_f][1], 0x42);
  });
}

// The half that matters more than the rejections: the restriction stops at the
// announcing record. Everything after it is read once the order is known, so it
// carries no restriction — and this schema, which is nothing but
// order-dependent fields after the announcement, has to both compile and read.
TEST(ByteOrderAxisRead, TheRestrictionDoesNotReachPastTheAnnouncingRecord) {
  PREPARE_INPUT_FILE({
    write_bytes(file, std::array<u8, 12>{'M', 'M', 0x00, 0x2a, 0xca, 0xfe, 0xd0, 0x0d,
                                         0x01, 0x2c, 0x01, 0x2c});
  });

  FIELD_LIST_SCHEMA = tiff_with_order_dependent_siblings;

  FIELD_LIST_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    EXPECT_EQ(fields["magic"_f], 42);
    EXPECT_EQ(fields["ifd_offset"_f], 0xcafed00du);
    EXPECT_EQ(fields["resolution"_f][0], 300);
    EXPECT_EQ(fields["resolution"_f][1], 300);
  });
}

// Split from the case below deliberately: one test asserting both passes with
// the order reference threaded only halfway, and this is the half that breaks
// first — entry_count is read by the *inner* fold, after the announcement's
// reader has written a cell the outer fold owns.
TEST(ByteOrderAxisRead, ANestedAnnouncementGovernsTheContainersLaterSiblings) {
  PREPARE_INPUT_FILE({
    write_bytes(file, std::array<u8, 8>{'M', 'M', 0x01, 0x2c, 0xca, 0xfe, 0xd0, 0x0d});
  });

  FIELD_LIST_SCHEMA = nested_once;

  FIELD_LIST_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["ifd"_f]["entry_count"_f], 300);
  });
}

TEST(ByteOrderAxisRead, ANestedAnnouncementGovernsWhatFollowsTheContainer) {
  PREPARE_INPUT_FILE({
    write_bytes(file, std::array<u8, 8>{'M', 'M', 0x01, 0x2c, 0xca, 0xfe, 0xd0, 0x0d});
  });

  FIELD_LIST_SCHEMA = nested_once;

  FIELD_LIST_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["next_ifd"_f], 0xcafed00du);
  });
}

TEST(ByteOrderAxisRead, AnAnnouncementTwoRecordsDeepResolvesTheSameWay) {
  PREPARE_INPUT_FILE({
    write_bytes(file, std::array<u8, 12>{'M', 'M', 0x01, 0x2c, 0xca, 0xfe, 0xd0, 0x0d,
                                         0xde, 0xad, 0xbe, 0xef});
  });

  FIELD_LIST_SCHEMA = nested_twice;

  FIELD_LIST_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    EXPECT_EQ(fields["first"_f]["ifd"_f]["entry_count"_f], 300);
    EXPECT_EQ(fields["first"_f]["next_ifd"_f], 0xcafed00du);
    EXPECT_EQ(fields["trailer"_f], 0xdeadbeefu);
  });
}
