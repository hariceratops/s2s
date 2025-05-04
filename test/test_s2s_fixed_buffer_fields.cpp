#include <gtest/gtest.h>
#include "../single_header/s2s.hpp"
#include "s2s_test_utils.hpp"


using namespace s2s_literals;

TEST(MetaStructReadTest, FixedBufferFieldsFromBinaryFile) {
  {
    std::ofstream file("test_input.bin", std::ios::out | std::ios::binary);
    constexpr std::size_t str_len = 10;
    const u8 str[] = "foo in bar";
    const u32 u32_arr[] = {0xdeadbeef, 0xcafed00d, 0xbeefbeef};
    file.write(reinterpret_cast<const char*>(&str), str_len + 1);
    file.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
  }

  FIELD_LIST_SCHEMA = 
    s2s::struct_field_list<
      // c_s2s::str_field<"a", 10>,
      s2s::fixed_string_field<"b", 10>,
      s2s::fixed_array_field<"c", u32, 3>
    >;

  []{
    std::ifstream file("test_input.bin", std::ios::in | std::ios::binary); \
    auto result = s2s::struct_cast_le<test_field_list>(file); \
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    std::string_view actual_sv{fields["b"_f].data()};
    std::string_view expected_sv{"foo in bar"};
    std::array<u32, 3> expected_arr{0xdeadbeef, 0xcafed00d, 0xbeefbeef};
    ASSERT_EQ(actual_sv, expected_sv);
    ASSERT_EQ(fields["c"_f], expected_arr);
  }();
}

TEST(MetaStructReadTest, MultiDimensionalFixedBufferFieldFromBinaryFile) {
  {
    std::ofstream ofs("test_input.bin", std::ios::out | std::ios::binary);
    const u32 u32_arr[3][3] = { 
      {0xdeadbeef, 0xcafed00d, 0xbeefbeef},
      {0xdeadbeef, 0xcafed00d, 0xbeefbeef}, 
      {0xdeadbeef, 0xcafed00d, 0xbeefbeef} 
    };
    ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
  }

  FIELD_LIST_SCHEMA = 
    s2s::struct_field_list<
      s2s::fixed_array_field<"arr", std::array<u32, 3>, 3>
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    ASSERT_EQ(
      fields["arr"_f],
      (std::array<std::array<u32, 3>, 3> {{
        {0xdeadbeef, 0xcafed00d, 0xbeefbeef},
        {0xdeadbeef, 0xcafed00d, 0xbeefbeef},
        {0xdeadbeef, 0xcafed00d, 0xbeefbeef}
      }})
    );
  });
}

