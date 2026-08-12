#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// Relocated from test/runtime/variable_buffer_fields.cpp. A length computed
// from several fields is a size-axis form, not a property of vec_field, so it
// waits here for 037 rather than staying with the descriptor tests.
TEST(SizeAxisRead, ReadsAVectorSizedByAComputationOverTwoFields) {
  []() {
    constexpr std::size_t row = 5;
    constexpr std::size_t col = 2;
    const u32 u32_arr[] = {
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d,
      0xdeadbeef, 0xcafed00d
    };
    std::ofstream file("test_input.bin", std::ios::out | std::ios::binary);
    file.write(reinterpret_cast<const char*>(&row), sizeof(row));
    file.write(reinterpret_cast<const char*>(&col), sizeof(col));
    file.write(reinterpret_cast<const char*>(&u32_arr), sizeof(u32_arr));
    file.close();
  }();

  auto size_from_rc = [](auto r, auto c) { return r * c; };
  FIELD_LIST_SCHEMA =
    s2s::struct_field_list<
      s2s::basic_field<"row", std::size_t, s2s::field_size<s2s::fixed<8>>>,
      s2s::basic_field<"col", std::size_t, s2s::field_size<s2s::fixed<8>>>,
      s2s::vec_field<
        "flat_vec",
        u32,
        s2s::field_size<
          s2s::len_from_fields<size_from_rc, s2s::with_fields<"row", "col">>
        >
      >
    >;

  FIELD_LIST_LE_READ_CHECK({
    ASSERT_TRUE(result.has_value());
    auto fields = *result;
    ASSERT_EQ(fields["row"_f], 5);
    ASSERT_EQ(fields["col"_f], 2);
    ASSERT_EQ(fields["flat_vec"_f].size(), 10);
    ASSERT_EQ(fields["flat_vec"_f], (std::vector<u32>{0xdeadbeef, 0xcafed00d,
                                                     0xdeadbeef, 0xcafed00d,
                                                     0xdeadbeef, 0xcafed00d,
                                                     0xdeadbeef, 0xcafed00d,
                                                     0xdeadbeef, 0xcafed00d}));
  });
}
