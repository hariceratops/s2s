// The byte-order axis, read side — real-stream tier.
// Spec:   dev/specs/byte-order-is-data-not-a-template-parameter.md
// Design: dev/design/byte-order-is-data-not-a-template-parameter.md §10
//
// Routing is by constant-evaluability (dev/specs/compile-time-test-tier.md):
// anything holding a real stream is GoogleTest and lives here; the
// constant-evaluable mirror is byte_order_axis_read_ct.cpp.

#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"

using namespace s2s_literals;

// TODO(054): the TIFF-shaped schema against a real input file, both II and MM.
// TODO(054): a marker matching neither form fails with validation_failure.
// TODO(056): the over-reach guard against a real stream.
// TODO(057): nesting one and two levels deep, as separate cases.
TEST(ByteOrderAxisRead, Placeholder) {
  GTEST_SKIP() << "scaffold — filled in by issue 054";
}
