#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// TODO(issue 003): round-trip magic_string/number/byte_array; constraint_checker rejects a wrong magic value
TEST(WriteMagicFields, RoundTripsMagicFieldsAndRejectsViolations) {
  GTEST_SKIP() << "not implemented — see issues/003";
}
