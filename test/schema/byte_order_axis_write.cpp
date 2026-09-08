// The byte-order axis, write side — real-stream tier.
// Spec:   dev/specs/byte-order-is-data-not-a-template-parameter.md
// Design: dev/design/byte-order-is-data-not-a-template-parameter.md §10
//
// Separate from byte_order_axis_read.cpp rather than one round-trip: a
// round-trip stays green if both directions are wrong the same way.

#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"

using namespace s2s_literals;

// TODO(059): byte-for-byte round trip in both forms against a real stream.
// TODO(059): setting the marker changes the output bytes.
// TODO(059): the nested write case.
TEST(ByteOrderAxisWrite, Placeholder) {
  GTEST_SKIP() << "scaffold — filled in by issue 059";
}
