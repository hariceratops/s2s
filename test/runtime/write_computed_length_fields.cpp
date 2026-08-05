#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// TODO(issue 007): len_from_fields disagreement and shared-length fan-out contradiction are rejected
TEST(WriteComputedLengthFields, RejectsInconsistentComputedLengths) {
  GTEST_SKIP() << "not implemented — see issues/007";
}
