#include <cstdint>
#include <variant>
#include <vector>
#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// The run-time half of union_alternative_options_read_ct.cpp. The compile-time
// file carries the matrix against a constexpr_memstream; this exists because
// the path has to be live in a non-constexpr build too, per the project's
// convention that stream-touching code needs both forms.
//
// Scaffolded ahead of 050/051; the baseline below is real and passes today.

namespace {
using plain =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::basic_field<"n", u32, 4_B>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0x01, s2s::as_trivial<u32, 4_B>>,
          s2s::match_case<0x02, s2s::as_vec<u8, s2s::len_from_field<"n">>>
        >
      >
    >
  >;

// A constraint on an as_vec alternative is checked against std::vector<u8>, the
// tag's resulting type, not against u8. See union_alternative_options_read_ct
// for the full matrix; this file keeps the same schema so the two tiers can be
// read side by side.
struct even_length {
  constexpr auto operator()(const std::vector<u8>& payload) const -> bool {
    return payload.size() % 2 == 0;
  }
};

using constrained =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::basic_field<"n", u32, 4_B>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0x01, s2s::as_trivial<u32, 4_B, s2s::lte{99u}>>,
          s2s::match_case<0x02, s2s::as_vec<u8, s2s::len_from_field<"n">, even_length{}>>,
          s2s::match_case<0x03, s2s::as_trivial<u16, 2_B>>
        >
      >
    >
  >;

auto stream_with(u32 tag, u32 n, const std::vector<u8>& payload) -> std::stringstream {
  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  stream.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
  stream.write(reinterpret_cast<const char*>(&n), sizeof(n));
  stream.write(reinterpret_cast<const char*>(payload.data()),
               static_cast<std::streamsize>(payload.size()));
  return stream;
}
} /* namespace */

TEST(UnionAlternativeOptions, ReadsAnAlternativeCarryingNoOptions) {
  auto stream = stream_with(0x01, 0, {0x2a, 0x00, 0x00, 0x00});

  auto result = s2s::struct_cast_le<plain>(stream);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(std::get<u32>((*result)["body"_f]), 42u);
}

TEST(UnionAlternativeOptions, ReadsAContainerAlternativeCarryingNoOptions) {
  auto stream = stream_with(0x02, 3, {0xaa, 0xbb, 0xcc});

  auto result = s2s::struct_cast_le<plain>(stream);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(std::get<std::vector<u8>>((*result)["body"_f]).size(), 3u);
}

TEST(UnionAlternativeOptions, ReadsAnAlternativeThatSatisfiesItsConstraint) {
  auto stream = stream_with(0x01, 0, {0x2a, 0x00, 0x00, 0x00});

  auto result = s2s::struct_cast_le<constrained>(stream);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(std::get<u32>((*result)["body"_f]), 42u);
}

// Non-vacuity, read side: deleting read_variant_impl's check must turn this red
// and leave union_alternative_options_write green.
TEST(UnionAlternativeOptions, RejectsAnAlternativeThatViolatesItsConstraint) {
  auto stream = stream_with(0x01, 0, {0xc8, 0x00, 0x00, 0x00});

  auto result = s2s::struct_cast_le<constrained>(stream);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().failure_reason, s2s::error_reason::validation_failure);
  EXPECT_EQ(result.error().failed_at, "body");
}

TEST(UnionAlternativeOptions, LeavesAnAlternativeThatDeclaredNoConstraintAlone) {
  auto stream = stream_with(0x03, 0, {0xc8, 0x00});

  auto result = s2s::struct_cast_le<constrained>(stream);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(std::get<u16>((*result)["body"_f]), 200u);
}

TEST(UnionAlternativeOptions, ChecksAContainerConstraintAgainstTheWholeContainer) {
  auto rejected = stream_with(0x02, 3, {0xaa, 0xbb, 0xcc});
  auto rejected_result = s2s::struct_cast_le<constrained>(rejected);

  ASSERT_FALSE(rejected_result.has_value());
  EXPECT_EQ(rejected_result.error().failure_reason, s2s::error_reason::validation_failure);
  EXPECT_EQ(rejected_result.error().failed_at, "body");

  auto accepted = stream_with(0x02, 4, {0xaa, 0xbb, 0xcc, 0xdd});
  auto accepted_result = s2s::struct_cast_le<constrained>(accepted);

  ASSERT_TRUE(accepted_result.has_value());
  EXPECT_EQ(std::get<std::vector<u8>>((*accepted_result)["body"_f]).size(), 4u);
}

//
// TODO(051): below-bound / at-bound / over-bound for as_vec, as_string and
// as_vec_of_records, asserting excessive_length on the last.
//
// TODO(051): a bound that is not a whole multiple of sizeof(element).
