#include <gtest/gtest.h>
#include "../single_header/s2s.hpp"
#include "s2s_test_utils.hpp"


using namespace s2s_literals;

TEST(MetaStructTest, ReadArrayOfRecordsFromBinaryFile) {
  // Write binary test file
  {
    std::ofstream ofs("test_input.bin", std::ios::out | std::ios::binary);
    const u32 u32_arr[3][2] = {
      {0xdeadbeef, 0xbeefbeef},
      {0xdeadbeef, 0xbeefbeef},
      {0xdeadbeef, 0xbeefbeef}
    };
    ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
    ofs.close();
  }

  using test_struct =
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>
    >;

  FIELD_LIST_SCHEMA =
    s2s::struct_field_list<
      s2s::array_of_records<"records", test_struct, 3>
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    auto records = fields["records"_f];
    for (const auto& record : records) {
      EXPECT_EQ(record["a"_f], 0xdeadbeef);
      EXPECT_EQ(record["b"_f], 0xbeefbeef);
    }
  });
}

