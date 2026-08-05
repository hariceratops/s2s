#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// TODO(issue 001): round-trip basic_field trivials in both byte orders, runtime stream path
TEST(WriteTrivialFields, RoundTripsTrivialFields) {
  GTEST_SKIP() << "not implemented — see issues/001";
}
