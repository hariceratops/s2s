#include <cstdint>
#include <sstream>
#include <variant>
#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// The run-time half of union_constraint_write_ct.cpp. See union_constraint_
// read_ct.cpp for why the union-level constraint gets its own pair rather than
// joining the per-alternative one.

namespace {
struct even_body {
  constexpr auto operator()(const std::variant<u32, u16>& body) const -> bool {
    return std::visit([](auto value) { return value % 2 == 0; }, body);
  }
};

using plain =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0x01, s2s::as_trivial<u32, 4_B>>,
          s2s::match_case<0x02, s2s::as_trivial<u16, 2_B>>
        >
      >
    >
  >;

using constrained =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0x01, s2s::as_trivial<u32, 4_B>>,
          s2s::match_case<0x02, s2s::as_trivial<u16, 2_B>>
        >
      >,
      even_body{}
    >
  >;
} /* namespace */

TEST(UnionConstraintWrite, WritesAVarianceDeclaringNoConstraint) {
  // Only "body" is assigned: "tag" is the discriminant, derived from the held
  // alternative rather than stored, and 043 made it non-assignable.
  plain obj{};
  obj["body"_f] = 43u;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::stream_cast_le<plain>(stream, obj);

  ASSERT_TRUE(written.has_value());
  EXPECT_EQ(stream.str().size(), 8u);
}

TEST(UnionConstraintWrite, WritesAResolvedVariantThatSatisfiesTheUnionConstraint) {
  constrained obj{};
  obj["body"_f] = 42u;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::stream_cast_le<constrained>(stream, obj);

  ASSERT_TRUE(written.has_value());
  EXPECT_EQ(static_cast<u8>(stream.str()[4]), u8{0x2a});
}

// Non-vacuity: the same value writes fine through `plain`.
TEST(UnionConstraintWrite, RejectsAResolvedVariantThatViolatesTheUnionConstraint) {
  constrained obj{};
  obj["body"_f] = 43u;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::stream_cast_le<constrained>(stream, obj);

  ASSERT_FALSE(written.has_value());
  EXPECT_EQ(written.error().failure_reason, s2s::error_reason::validation_failure);
  EXPECT_EQ(written.error().failed_at, "body");
  // The fold validates a field before writing it, so the union contributed no
  // bytes: what reached the stream is the discriminant ahead of it.
  EXPECT_EQ(stream.str().size(), 4u);
}

TEST(UnionConstraintWrite, AppliesTheUnionConstraintToEveryAlternative) {
  constrained accepted{};
  accepted["body"_f] = u16{200};

  std::stringstream accepted_stream(std::ios::in | std::ios::out | std::ios::binary);
  auto accepted_written = s2s::stream_cast_le<constrained>(accepted_stream, accepted);

  ASSERT_TRUE(accepted_written.has_value());
  EXPECT_EQ(static_cast<u8>(accepted_stream.str()[4]), u8{0xc8});

  constrained rejected{};
  rejected["body"_f] = u16{201};

  std::stringstream rejected_stream(std::ios::in | std::ios::out | std::ios::binary);
  auto rejected_written = s2s::stream_cast_le<constrained>(rejected_stream, rejected);

  ASSERT_FALSE(rejected_written.has_value());
  EXPECT_EQ(rejected_written.error().failure_reason, s2s::error_reason::validation_failure);
  EXPECT_EQ(rejected_written.error().failed_at, "body");
}
