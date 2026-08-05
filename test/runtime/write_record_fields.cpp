#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// TODO(issue 006): round-trip struct_field, array_of_records, vector_of_records via recursive write
TEST(WriteRecordFields, RoundTripsRecordFields) {
  GTEST_SKIP() << "not implemented — see issues/006";
}
