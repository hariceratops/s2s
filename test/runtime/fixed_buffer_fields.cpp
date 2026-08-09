#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


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

// No read test exercised a foreign-endian buffer, which is what let
// read_foreign_buffer byteswap the container instead of its elements.
TEST(MetaStructReadTest, ForeignEndianFixedBufferIsSwappedPerElement) {
  {
    std::ofstream file("test_input.bin", std::ios::out | std::ios::binary);
    // fixed_string_field<N> is sized fixed<N + 1>, so the terminator is on the wire.
    const u8 be_bytes[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 'a', 'b', 'c', 'd', '\0'};
    file.write(reinterpret_cast<const char*>(be_bytes), sizeof(be_bytes));
  }

  FIELD_LIST_SCHEMA =
    s2s::struct_field_list<
      s2s::fixed_array_field<"arr", u16, 3>,
      s2s::fixed_string_field<"name", 4>
    >;

  FIELD_LIST_BE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    ASSERT_EQ(fields["arr"_f], (std::array<u16, 3>{0x1122, 0x3344, 0x5566}));
    // char has no byte order: the string must not come back reversed.
    ASSERT_EQ(std::string_view(fields["name"_f].data()), std::string_view("abcd"));
  });
}

// The nested case only ever ran host-endian, so the per-element swap never
// had to descend into the inner array.
TEST(MetaStructReadTest, ForeignEndianMultiDimensionalFixedBuffer) {
  {
    std::ofstream file("test_input.bin", std::ios::out | std::ios::binary);
    const u8 be_bytes[] = {
      0x11, 0x22, 0x33, 0x44,
      0x55, 0x66, 0x77, 0x88,
      0x99, 0xaa, 0xbb, 0xcc,
      0xdd, 0xee, 0xff, 0x00
    };
    file.write(reinterpret_cast<const char*>(be_bytes), sizeof(be_bytes));
  }

  FIELD_LIST_SCHEMA =
    s2s::struct_field_list<
      s2s::fixed_array_field<"arr", std::array<u16, 2>, 4>
    >;

  FIELD_LIST_BE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    ASSERT_EQ(
      fields["arr"_f],
      (std::array<std::array<u16, 2>, 4> {{
        {0x1122, 0x3344},
        {0x5566, 0x7788},
        {0x99aa, 0xbbcc},
        {0xddee, 0xff00}
      }})
    );
  });
}

