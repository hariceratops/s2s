#include <array>
#include <bit>
#include <cstdint>
#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"

namespace {
constexpr auto byte_reversed(float v) -> float {
  return std::bit_cast<float>(std::byteswap(std::bit_cast<std::uint32_t>(v)));
}
} // namespace


using namespace s2s_literals;

TEST(TrivialRead, ReadsTrivialFieldsInBothByteOrders) {
  PREPARE_INPUT_FILE({
    u32 a = 0xdeadbeef;
    u32 b = 0xcafed00d;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
  });

  FIELD_LIST_SCHEMA =
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, 4_B>,
      s2s::basic_field<"b", u32, 4_B>
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    if (result) {
      auto fields = *result;
      EXPECT_EQ(fields["a"_f], 0xdeadbeef);
      EXPECT_EQ(fields["b"_f], 0xcafed00d);
    }
  });

  FIELD_LIST_BE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    if (result) {
      auto fields = *result;
      EXPECT_EQ(fields["a"_f], 0xefbeadde);
      EXPECT_EQ(fields["b"_f], 0x0dd0feca);
    }
  });
}

// std::byteswap has no floating-point overload, and a schema containing a
// float reaches both the host and foreign arms of read_impl once byte order
// is a runtime value, so both directions have to actually work on one.
TEST(TrivialRead, ReadsFloatFieldsInBothByteOrders) {
  PREPARE_INPUT_FILE({
    float a = 1.5f;
    float b0 = 2.5f;
    float b1 = -3.25f;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b0), sizeof(b0));
    file.write(reinterpret_cast<const char*>(&b1), sizeof(b1));
  });

  FIELD_LIST_SCHEMA =
    s2s::struct_field_list<
      s2s::basic_field<"a", float, 4_B>,
      s2s::fixed_array_field<"b", float, 2>
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    EXPECT_EQ(fields["a"_f], 1.5f);
    EXPECT_EQ(fields["b"_f][0], 2.5f);
    EXPECT_EQ(fields["b"_f][1], -3.25f);
  });

  FIELD_LIST_BE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    EXPECT_EQ(fields["a"_f], byte_reversed(1.5f));
    EXPECT_EQ(fields["b"_f][0], byte_reversed(2.5f));
    EXPECT_EQ(fields["b"_f][1], byte_reversed(-3.25f));
  });
}

TEST(TrivialRead, RejectsAFieldThatViolatesItsConstraint) {
  PREPARE_INPUT_FILE({
    u32 a = 0xdeadbeef;
    u32 b = 0xdeadbeef;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
  });

  FIELD_LIST_SCHEMA =
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, 4_B, s2s::eq(0xdeadbeef)>,
      s2s::basic_field<"b", u32, 4_B, s2s::eq(0xcafed00d)>
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_FALSE(result.has_value());
    auto err = result.error();
    EXPECT_EQ(err.failure_reason, s2s::error_reason::validation_failure);
    EXPECT_EQ(err.failed_at, "b");
  });
}

TEST(TrivialRead, ReportsExhaustionOnTheFieldThatRanOut) {
  PREPARE_INPUT_FILE({
    u32 a = 0xdeadbeef;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.close();
  });

  FIELD_LIST_SCHEMA =
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, 4_B>,
      s2s::basic_field<"b", u32, 4_B>
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_FALSE(result.has_value());
    auto err = result.error();
    EXPECT_EQ(err.failure_reason, s2s::error_reason::buffer_exhaustion);
    EXPECT_EQ(err.failed_at, "b");
  });
}
