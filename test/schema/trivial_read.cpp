#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


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
