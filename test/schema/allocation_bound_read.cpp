#include <cstdint>
#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// The run-time half of allocation_bound_read_ct.cpp. The compile-time cases
// cover the matrix against a constexpr_memstream; these exist because the code
// path has to be live in a non-constexpr build too, per the project's
// convention that stream-touching code needs both forms.

namespace {
using bounded_vec =
  s2s::struct_field_list<
    s2s::basic_field<"n", u32, 4_B>,
    s2s::vec_field<"v", u16, s2s::len_from_field<"n">, s2s::max_bytes<8>>
  >;

using undeclared =
  s2s::struct_field_list<
    s2s::basic_field<"n", u32, 4_B>,
    s2s::vec_field<"v", u16, s2s::len_from_field<"n">>
  >;

auto stream_with(u32 count, std::size_t payload_bytes) -> std::stringstream {
  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  stream.write(reinterpret_cast<const char*>(&count), sizeof(count));
  const std::string payload(payload_bytes, '\0');
  stream.write(payload.data(), static_cast<std::streamsize>(payload.size()));
  return stream;
}
} /* namespace */

TEST(AllocationBound, RejectsALengthOverItsDeclaredBound) {
  // The payload is present, so a rejection is the bound talking rather than
  // the stream running out.
  auto stream = stream_with(5, 10);

  auto result = s2s::struct_cast_le<bounded_vec>(stream);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().failure_reason, s2s::error_reason::excessive_length);
  EXPECT_EQ(result.error().failed_at, "v");
}

TEST(AllocationBound, AcceptsALengthExactlyAtItsDeclaredBound) {
  auto stream = stream_with(4, 8);

  auto result = s2s::struct_cast_le<bounded_vec>(stream);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ((*result)["v"_f].size(), 4u);
}

// The headline behaviour: a schema declaring nothing is still protected.
// 0x01000000 u16 elements is 32 MiB, over the 16 MiB default.
TEST(AllocationBound, RejectsAnAbsurdLengthWithNothingDeclared) {
  auto stream = stream_with(0x01000000u, 0);

  auto result = s2s::struct_cast_le<undeclared>(stream);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().failure_reason, s2s::error_reason::excessive_length);
}
