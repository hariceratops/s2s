#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

TEST(RecordRead, ReadsANestedRecord) {
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

TEST(RecordRead, ReadsAnArrayOfRecords) {
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

TEST(RecordRead, ReadsAVectorOfRecords) {
  []() {
    constexpr std::size_t vec_len = 3;
    const u32 u32_arr[3][2] = { 
      {0xdeadbeef, 0xbeefbeef},
      {0xdeadbeef, 0xbeefbeef}, 
      {0xdeadbeef, 0xbeefbeef} 
    };
    std::ofstream file("test_input.bin", std::ios::out | std::ios::binary);
    file.write(reinterpret_cast<const char*>(&vec_len), sizeof(vec_len));
    file.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
    file.close();
  }();

  using test_struct = 
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>
    >;

  FIELD_LIST_SCHEMA = 
    s2s::struct_field_list<
      s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
      s2s::vector_of_records<
        "records", 
        test_struct, 
        s2s::field_size<s2s::len_from_field<"len">>
      >
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


TEST(RecordRead, ReadsAVectorOfRecordsBigEndian) {
  []() {
    constexpr std::size_t vec_len = 0x0300000000000000;
    const u32 u32_arr[3][2] = { 
      {0xdeadbeef, 0xbeefbeef},
      {0xdeadbeef, 0xbeefbeef}, 
      {0xdeadbeef, 0xbeefbeef} 
    };
    std::ofstream file("test_input.bin", std::ios::out | std::ios::binary);
    file.write(reinterpret_cast<const char*>(&vec_len), sizeof(vec_len));
    file.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
    file.close();
  }();

  using test_struct = 
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>
    >;

  FIELD_LIST_SCHEMA = 
    s2s::struct_field_list<
      s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
      s2s::vector_of_records<
        "records", 
        test_struct, 
        s2s::field_size<s2s::len_from_field<"len">>
      >
    >;

  FIELD_LIST_BE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    auto records = fields["records"_f];
    for (const auto& record : records) {
      EXPECT_EQ(record["a"_f], 0xefbeadde);
      EXPECT_EQ(record["b"_f], 0xefbeefbe);
    }
  });
}
