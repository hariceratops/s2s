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

// Written a byte at a time rather than through an integer: the point of the
// test is which byte lands where, and reinterpreting an int would make the file
// depend on the host's own order.
auto write_header(std::ofstream& file, const char (&marker)[3],
                  u8 magic_hi, u8 magic_lo, const u8 (&offset)[4]) -> void {
  const u8 bytes[] = {
    static_cast<u8>(marker[0]), static_cast<u8>(marker[1]),
    magic_hi, magic_lo,
    offset[0], offset[1], offset[2], offset[3]
  };
  file.write(reinterpret_cast<const char*>(bytes), sizeof(bytes));
}
} /* namespace */

// The whole claim of the feature: two different byte sequences, one schema, one
// entry point taking no order, and the same values out of both.
TEST(ByteOrderAxisRead, AnIIHeaderMakesTheRestOfTheFileLittleEndian) {
  PREPARE_INPUT_FILE({
    write_header(file, "II", 0x2a, 0x00, {0x0d, 0xd0, 0xfe, 0xca});
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
    write_header(file, "MM", 0x00, 0x2a, {0xca, 0xfe, 0xd0, 0x0d});
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
    write_header(file, "XX", 0x2a, 0x00, {0x0d, 0xd0, 0xfe, 0xca});
  });

  FIELD_LIST_SCHEMA = tiff_header;

  FIELD_LIST_READ_CHECK({
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().failure_reason, s2s::error_reason::validation_failure);
    EXPECT_EQ(result.error().failed_at, "byte_order");
  });
}

// TODO(056): the over-reach guard against a real stream.
// TODO(057): nesting one and two levels deep, as separate cases.
