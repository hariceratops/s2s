#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// The run-time half of allocation_bound_read_ct.cpp. The compile-time cases
// cover the matrix against a constexpr_memstream; these exist because the code
// path has to be live in a non-constexpr build too, per the project's
// convention that stream-touching code needs both forms.

// TODO(047): a length over the field's declared max_bytes is rejected with the
// new reason, before any allocation, against a real stream.
TEST(AllocationBound, RejectsALengthOverItsDeclaredBound) {
  GTEST_SKIP() << "TODO(047): max_bytes does not exist yet";
}

// TODO(047): and exactly at the bound is accepted — inclusive.
TEST(AllocationBound, AcceptsALengthExactlyAtItsDeclaredBound) {
  GTEST_SKIP() << "TODO(047): max_bytes does not exist yet";
}

// TODO(047): the default protects a schema that declares nothing. This is the
// headline behaviour of the feature and the one a user gets without having
// thought about it.
TEST(AllocationBound, RejectsAnAbsurdLengthWithNothingDeclared) {
  GTEST_SKIP() << "TODO(047): the default ceiling does not exist yet";
}

// TODO(047): vector_of_records is bounded by count * sizeof(record_struct) —
// its memory footprint, which has no relationship to its wire size.
TEST(AllocationBound, BoundsARecordVectorByItsFootprint) {
  GTEST_SKIP() << "TODO(047): max_bytes does not exist yet";
}
