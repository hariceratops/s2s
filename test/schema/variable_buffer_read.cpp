#include <cstdint>
#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

TEST(VariableBufferRead, ReadsAStringSizedByItsLengthField) {
  PREPARE_INPUT_FILE({
    constexpr std::size_t str_len = 10;
    const u8 str[] = "foo in bar";
    file.write(reinterpret_cast<const char*>(&str_len), sizeof(str_len));
    file.write(reinterpret_cast<const char*>(&str), str_len);
  });

  FIELD_LIST_SCHEMA =
    s2s::struct_field_list<
      s2s::basic_field<"len", std::size_t, 8_B>,
      s2s::str_field<"str", s2s::len_from_field<"len">>
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    ASSERT_EQ(fields["str"_f].size(), 10);
    std::string_view expected{"foo in bar"};
    ASSERT_EQ(std::string_view{fields["str"_f]}, expected);
  });
}

TEST(VariableBufferRead, ReadsAVectorSizedByItsLengthField) {
  []() {
    constexpr std::size_t vec_len = 10;
    const u8 str[] = "foo in bar";
    const u32 u32_arr[] = {
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d
    };
    std::ofstream file("test_input.bin", std::ios::out | std::ios::binary);
    file.write(reinterpret_cast<const char*>(&vec_len), sizeof(vec_len));
    file.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
    file.write(reinterpret_cast<const char*>(&str), vec_len + 1);
  }();

  FIELD_LIST_SCHEMA =
    s2s::struct_field_list<
      s2s::basic_field<"len", std::size_t, 8_B>,
      s2s::vec_field<
        "vec",
        u32,
        s2s::len_from_field<"len">
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    ASSERT_EQ(fields["vec"_f].size(), 10);
    ASSERT_EQ(fields["vec"_f], (std::vector<u32>{0xdeadbeef, 0xcafed00d,
                                                 0xdeadbeef, 0xcafed00d,
                                                 0xdeadbeef, 0xcafed00d,
                                                 0xdeadbeef, 0xcafed00d,
                                                 0xdeadbeef, 0xcafed00d}));
  });
}

// todo recursive resize
// TEST_CASE("Test reading a meta_struct with aliased length pres2s::fixed md_buffer fields depending on multiple fields from binary file") {
//   auto size_from_rc = [](auto r, auto c) { return r * c; };
//
//   using var_buffer_struct = 
//     s2s::struct_field_list<
//       s2s::basic_field<"row", std::size_t, 8_B>,
//       s2s::basic_field<"col", std::size_t, 8_B>,
//       s2s::vec_field<"matrix", std::vector<u32>, from_fields<size_from_rc, "row", "col">>
//     >;
//
//   constexpr std::size_t row = 5;
//   constexpr std::size_t col = 2;
//   const u32 u32_arr[] = {
//     0xdeadbeef, 0xcafed00d,
//     0xdeadbeef, 0xcafed00d,
//     0xdeadbeef, 0xcafed00d,
//     0xdeadbeef, 0xcafed00d,
//     0xdeadbeef, 0xcafed00d
//   };
//
//   std::ofstream ofs("test_input_md_s2s::vec_fields.bin", std::ios::out | std::ios::binary);
//   ofs.write(reinterpret_cast<const char*>(&row), sizeof(row));
//   ofs.write(reinterpret_cast<const char*>(&col), sizeof(col));
//   ofs.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
//   ofs.close();
//
//   std::ifstream ifs("test_input_md_s2s::vec_fields.bin", std::ios::in | std::ios::binary);
//   // oops resizing recursively is not possible
//   // auto res = struct_cast<var_buffer_struct>(ifs);
//   ifs.close();
//  
//   // REQUIRE(res.has_value());
//   // auto fields = *res;
//   // REQUIRE(fields["row"_f] == 5);
//   // REQUIRE(fields["col"_f] == 2);
//   // REQUIRE(fields["matrix"_f].size() == 10);
//   // REQUIRE(fields["vec"_f] == std::vector<u32>{0xdeadbeef, 0xcafed00d, 
//   //                                             0xdeadbeef, 0xcafed00d,
//   //                                             0xdeadbeef, 0xcafed00d,
//   //                                             0xdeadbeef, 0xcafed00d, 
//   //                                             0xdeadbeef, 0xcafed00d});
// };


// The run-time half of the overflow gate. The rejection happens before the
// stream is touched for the payload, so this is about the path being live in a
// non-constexpr build rather than about stream behaviour.
TEST(VariableBufferRead, RejectsALengthWhoseByteCountWouldOverflow) {
  using test_field_list =
    s2s::struct_field_list<
      s2s::basic_field<"n", std::uint64_t, 8_B>,
      s2s::vec_field<"v", std::uint64_t, s2s::len_from_field<"n">>
    >;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  const std::uint64_t absurd = std::uint64_t{1} << 61;
  stream.write(reinterpret_cast<const char*>(&absurd), sizeof(absurd));

  auto result = s2s::struct_cast_le<test_field_list>(stream);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().failure_reason, s2s::error_reason::excessive_length);
  EXPECT_EQ(result.error().failed_at, "v");
}
