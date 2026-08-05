#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// TODO(issue 002): round-trip fixed_array_field, c_arr_field, fixed_string_field, c_str_field; element-wise byteswap
TEST(WriteFixedBufferFields, RoundTripsFixedSizeAggregates) {
  GTEST_SKIP() << "not implemented — see issues/002";
}
