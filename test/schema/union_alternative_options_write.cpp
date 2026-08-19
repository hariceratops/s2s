#include <cstdint>
#include <variant>
#include <vector>
#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// The run-time half of union_alternative_options_write_ct.cpp. See that file
// for why the write cases live apart from the read ones rather than sharing a
// round-trip: 050's two constraint call sites have to be shown to fail
// independently.

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

// The interaction 052 calls out as the one most likely to be got wrong: 0x01
// must be at most 99 *and* the resolved body must be even, and neither implies
// the other. Spelled as a functor because a std::variant is not a structural
// type and cannot be a template argument to eq{}.
struct even_body {
  constexpr auto operator()(const std::variant<u32, u16>& body) const -> bool {
    return std::visit([](auto value) { return value % 2 == 0; }, body);
  }
};

using both_constrained =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0x01, s2s::as_trivial<u32, 4_B, s2s::lte{99u}>>,
          s2s::match_case<0x02, s2s::as_trivial<u16, 2_B>>
        >
      >,
      even_body{}
    >
  >;
} /* namespace */

TEST(UnionAlternativeOptionsWrite, WritesAnAlternativeCarryingNoOptions) {
  // Only "body" is assigned: "tag" is the discriminant and "n" the length
  // target, both derived from the held alternative rather than stored, and 043
  // made them non-assignable so a schema author cannot contradict the
  // derivation.
  plain obj{};
  obj["body"_f] = 42u;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::stream_cast_le<plain>(stream, obj).has_value());
  EXPECT_EQ(stream.str(), std::string("\x01\x00\x00\x00\x00\x00\x00\x00\x2a\x00\x00\x00", 12));
}

TEST(UnionAlternativeOptionsWrite, WritesAnAlternativeThatSatisfiesItsConstraint) {
  constrained obj{};
  obj["body"_f] = 42u;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::stream_cast_le<constrained>(stream, obj).has_value());
  EXPECT_EQ(stream.str(), std::string("\x01\x00\x00\x00\x00\x00\x00\x00\x2a\x00\x00\x00", 12));
}

// Non-vacuity, write side: deleting write_variant_impl's check must turn this
// red and leave union_alternative_options_read green.
TEST(UnionAlternativeOptionsWrite, RejectsAnAlternativeThatViolatesItsConstraint) {
  constrained obj{};
  obj["body"_f] = 200u;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::stream_cast_le<constrained>(stream, obj);

  ASSERT_FALSE(written.has_value());
  EXPECT_EQ(written.error().failure_reason, s2s::error_reason::validation_failure);
  EXPECT_EQ(written.error().failed_at, "body");
  // The check runs before write_field, so the union contributed no bytes: what
  // reached the stream is the two fields ahead of it and nothing more.
  EXPECT_EQ(stream.str().size(), 8u);
}

TEST(UnionAlternativeOptionsWrite, LeavesAnAlternativeThatDeclaredNoConstraintAlone) {
  constrained obj{};
  obj["body"_f] = u16{200};

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::stream_cast_le<constrained>(stream, obj).has_value());
  EXPECT_EQ(stream.str(), std::string("\x03\x00\x00\x00\x00\x00\x00\x00\xc8\x00", 10));
}

TEST(UnionAlternativeOptionsWrite, ChecksAContainerConstraintAgainstTheWholeContainer) {
  constrained rejected{};
  rejected["n"_f] = 3;
  rejected["body"_f] = std::vector<u8>{0xaa, 0xbb, 0xcc};

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::stream_cast_le<constrained>(stream, rejected);

  ASSERT_FALSE(written.has_value());
  EXPECT_EQ(written.error().failure_reason, s2s::error_reason::validation_failure);

  constrained accepted{};
  accepted["n"_f] = 4;
  accepted["body"_f] = std::vector<u8>{0xaa, 0xbb, 0xcc, 0xdd};

  std::stringstream ok_stream(std::ios::in | std::ios::out | std::ios::binary);
  EXPECT_TRUE(s2s::stream_cast_le<constrained>(ok_stream, accepted).has_value());
}

TEST(UnionAlternativeOptionsWrite, WritesWhenBothConstraintsHold) {
  both_constrained obj{};
  obj["body"_f] = 42u;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::stream_cast_le<both_constrained>(stream, obj).has_value());
  EXPECT_EQ(stream.str(), std::string("\x01\x00\x00\x00\x2a\x00\x00\x00", 8));
}

// The two rejections below are indistinguishable by their error, and by the
// bytes on the stream — every alternative takes the union's id, so both report
// validation_failure at "body", and both stop after the discriminant. What
// separates them is the value: 43 satisfies lte{99u} and only the union
// constraint can reject it, 200 is even and only the alternative's can. Each
// therefore proves one check fired with the other passing.
TEST(UnionAlternativeOptionsWrite, RejectsOnTheUnionConstraintWhenTheAlternativeAccepts) {
  both_constrained obj{};
  obj["body"_f] = 43u;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::stream_cast_le<both_constrained>(stream, obj);

  ASSERT_FALSE(written.has_value());
  EXPECT_EQ(written.error().failure_reason, s2s::error_reason::validation_failure);
  EXPECT_EQ(written.error().failed_at, "body");
  EXPECT_EQ(stream.str().size(), 4u);
}

TEST(UnionAlternativeOptionsWrite, RejectsOnTheAlternativeConstraintWhenTheUnionAccepts) {
  both_constrained obj{};
  obj["body"_f] = 200u;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::stream_cast_le<both_constrained>(stream, obj);

  ASSERT_FALSE(written.has_value());
  EXPECT_EQ(written.error().failure_reason, s2s::error_reason::validation_failure);
  EXPECT_EQ(written.error().failed_at, "body");
  EXPECT_EQ(stream.str().size(), 4u);
}

//
// No write-side bound cases: a bound guards an allocation driven by a wire
// length, and the write path serialises a container the caller already owns.
