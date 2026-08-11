#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

TEST(RecordRead, ReadsANestedRecordFromBinaryFile) {
  PREPARE_INPUT_FILE({
    u32 a = 0xdeadbeef;
    u32 b = 0xcafed00d;
    u32 x = 0xbeefbeef;
    u32 y = 0xdeadbeef;
    file.write(reinterpret_cast<const char*>(&a), sizeof(a));
    file.write(reinterpret_cast<const char*>(&b), sizeof(b));
    file.write(reinterpret_cast<const char*>(&x), sizeof(x));
    file.write(reinterpret_cast<const char*>(&y), sizeof(y));
  });

  FIELD_LIST_SCHEMA =
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::struct_field<
        "c",
        s2s::struct_field_list<
          s2s::basic_field<"x", u32, s2s::field_size<s2s::fixed<4>>>,
          s2s::basic_field<"y", u32, s2s::field_size<s2s::fixed<4>>>
        >
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    if (result) {
      auto fields = *result;
      EXPECT_EQ(fields["a"_f], 0xdeadbeef);
      EXPECT_EQ(fields["b"_f], 0xcafed00d);
      EXPECT_EQ(fields["c"_f]["x"_f], 0xbeefbeef);
      EXPECT_EQ(fields["c"_f]["y"_f], 0xdeadbeef);
    }
  });
}
