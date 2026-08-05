#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// TODO(issue 009): round-trip type_switch (derived discriminant) and if_else_ladder (verified) variance
TEST(WriteUnionFields, RoundTripsUnionFields) {
  GTEST_SKIP() << "not implemented — see issues/009";
}
