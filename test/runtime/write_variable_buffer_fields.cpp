#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// TODO(issue 004): round-trip vec_field and str_field with lengths derived from container size; reject width overflow
TEST(WriteVariableBufferFields, RoundTripsLengthPrefixedFields) {
  GTEST_SKIP() << "not implemented — see issues/004";
}
