#include <cstdint>
#include <sstream>
#include <variant>
#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// The run-time half of union_constraint_read_ct.cpp. The compile-time file
// carries the same matrix against a constexpr_memstream; this exists because
// the path has to be live in a non-constexpr build too, per the project's
// convention that stream-touching code needs both forms.

namespace {
// Spelled as a functor, not as one of the built-in comparisons: the value being
// constrained is a std::variant, which is not a structural type and so cannot
// be a template argument to eq{} at all.
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

auto stream_with(u32 tag, const std::vector<u8>& body) -> std::stringstream {
  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  stream.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
  stream.write(reinterpret_cast<const char*>(body.data()),
               static_cast<std::streamsize>(body.size()));
  return stream;
}
} /* namespace */

TEST(UnionConstraint, ReadsAVarianceDeclaringNoConstraint) {
  auto stream = stream_with(0x01, {0x2b, 0x00, 0x00, 0x00});

  auto result = s2s::struct_cast_le<plain>(stream);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(std::get<u32>((*result)["body"_f]), 43u);
}

TEST(UnionConstraint, ReadsAResolvedVariantThatSatisfiesTheUnionConstraint) {
  auto stream = stream_with(0x01, {0x2a, 0x00, 0x00, 0x00});

  auto result = s2s::struct_cast_le<constrained>(stream);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(std::get<u32>((*result)["body"_f]), 42u);
}

// Non-vacuity: the same bytes read fine through `plain`, so the rejection is
// the constraint and nothing else.
TEST(UnionConstraint, RejectsAResolvedVariantThatViolatesTheUnionConstraint) {
  auto stream = stream_with(0x01, {0x2b, 0x00, 0x00, 0x00});

  auto result = s2s::struct_cast_le<constrained>(stream);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().failure_reason, s2s::error_reason::validation_failure);
  EXPECT_EQ(result.error().failed_at, "body");
}

// The point of a union-level constraint: it holds whichever alternative was
// resolved, and neither alternative here declares one of its own.
TEST(UnionConstraint, AppliesTheUnionConstraintToEveryAlternative) {
  auto accepted = stream_with(0x02, {0xc8, 0x00});
  auto accepted_result = s2s::struct_cast_le<constrained>(accepted);

  ASSERT_TRUE(accepted_result.has_value());
  EXPECT_EQ(std::get<u16>((*accepted_result)["body"_f]), u16{200});

  auto rejected = stream_with(0x02, {0xc9, 0x00});
  auto rejected_result = s2s::struct_cast_le<constrained>(rejected);

  ASSERT_FALSE(rejected_result.has_value());
  EXPECT_EQ(rejected_result.error().failure_reason, s2s::error_reason::validation_failure);
  EXPECT_EQ(rejected_result.error().failed_at, "body");
}
