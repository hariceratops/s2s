#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// TODO(issue 008): round-trip maybe/parse_if; reject presence mismatch in both directions
TEST(WriteOptionalFields, RoundTripsOptionalsAndRejectsPresenceMismatch) {
  GTEST_SKIP() << "not implemented — see issues/008";
}
