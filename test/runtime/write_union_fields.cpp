#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

namespace {
using inner_1 =
  s2s::struct_field_list<
    s2s::basic_field<"x", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"y", u32, s2s::field_size<s2s::fixed<4>>>
  >;
using inner_2 =
  s2s::struct_field_list<
    s2s::basic_field<"p", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"q", u32, s2s::field_size<s2s::fixed<4>>>
  >;

using switch_schema =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0xcafed00d, s2s::as_struct<inner_1>>,
          s2s::match_case<0xdeadbeef, s2s::as_struct<inner_2>>
        >
      >
    >
  >;

auto make_inner_2(u32 p, u32 q) -> inner_2 {
  inner_2 obj{};
  obj["p"_f] = p;
  obj["q"_f] = q;
  return obj;
}

auto pick_by_sum = [](auto a, auto b) { return a + b; };

using computed_schema =
  s2s::struct_field_list<
    s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::compute<pick_by_sum, u32, s2s::with_fields<"a", "b">>,
        s2s::type_switch<
          s2s::match_case<100, s2s::as_trivial<u32, s2s::field_size<s2s::fixed<4>>>>,
          s2s::match_case<200, s2s::as_trivial<i32, s2s::field_size<s2s::fixed<4>>>>
        >
      >
    >
  >;

auto sum_is_small = [](auto a, auto b) { return a + b < 100; };
auto sum_is_large = [](auto a, auto b) { return a + b >= 100; };

using ladder_schema =
  s2s::struct_field_list<
    s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::type_if_else<
          s2s::branch<
            s2s::predicate<sum_is_small, s2s::with_fields<"a", "b">>,
            s2s::as_trivial<u32, s2s::field_size<s2s::fixed<4>>>
          >,
          s2s::branch<
            s2s::predicate<sum_is_large, s2s::with_fields<"a", "b">>,
            s2s::as_trivial<i32, s2s::field_size<s2s::fixed<4>>>
          >
        >
      >
    >
  >;
} /* namespace */

TEST(WriteUnionFields, DerivesTheDiscriminantFromTheHeldAlternative) {
  using test_field_list = switch_schema;

  switch_schema obj{};
  obj["body"_f] = make_inner_2(0x11223344, 0x55667788);

  std::stringstream be(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::struct_write_be<test_field_list>(be, obj).has_value());
  // 0xdeadbeef is the match_case value for inner_2 — never stored, derived.
  EXPECT_EQ(be.str(), std::string("\xde\xad\xbe\xef\x11\x22\x33\x44\x55\x66\x77\x88", 12));
}

TEST(WriteUnionFields, RoundTripsSwitchUnionsInBothByteOrders) {
  using test_field_list = switch_schema;

  switch_schema obj{};
  obj["body"_f] = make_inner_2(0xdeadbeef, 0xcafed00d);

  FIELD_LIST_LE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["tag"_f], 0xdeadbeef);
    const auto held = std::get<inner_2>((*result)["body"_f]);
    EXPECT_EQ(held["p"_f], 0xdeadbeef);
    EXPECT_EQ(held["q"_f], 0xcafed00d);
  });

  FIELD_LIST_BE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    const auto held = std::get<inner_2>((*result)["body"_f]);
    EXPECT_EQ(held["q"_f], 0xcafed00d);
  });
}

// The alternative is a record, so 006's recursion has to carry through.
TEST(WriteUnionFields, RecursesIntoRecordAlternatives) {
  using test_field_list = switch_schema;

  inner_1 held{};
  held["x"_f] = 0x1111;
  held["y"_f] = 0x2222;

  switch_schema obj{};
  obj["body"_f] = held;

  FIELD_LIST_BE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["tag"_f], 0xcafed00d);
    EXPECT_EQ(std::get<inner_1>((*result)["body"_f])["y"_f], 0x2222u);
  });
}

TEST(WriteUnionFields, RoundTripsLadderUnionsWhenTheHeldAlternativeAgrees) {
  using test_field_list = ladder_schema;

  ladder_schema obj{};
  obj["a"_f] = 10;
  obj["b"_f] = 20;
  obj["body"_f] = static_cast<u32>(0xdeadbeef);

  FIELD_LIST_LE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::get<u32>((*result)["body"_f]), 0xdeadbeef);
  });
}

// A ladder branch is an arbitrary predicate with no inverse, so the held
// alternative can only be checked against what the reader will select.
TEST(WriteUnionFields, RejectsALadderAlternativeThePredicatesWouldNotSelect) {
  using test_field_list = ladder_schema;

  ladder_schema obj{};
  obj["a"_f] = 10;
  obj["b"_f] = 20;
  obj["body"_f] = static_cast<i32>(-1);

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::struct_write_le<test_field_list>(stream, obj);
  ASSERT_FALSE(written.has_value());
  EXPECT_EQ(written.error().failure_reason, s2s::error_reason::validation_failure);
  EXPECT_EQ(written.error().failed_at, "body");
  EXPECT_EQ(stream.str().size(), 8u);
}

TEST(WriteUnionFields, RejectsAComputedSwitchAlternativeThatDisagrees) {
  using test_field_list = computed_schema;

  computed_schema obj{};
  obj["a"_f] = 50;
  obj["b"_f] = 50;
  obj["body"_f] = static_cast<i32>(-1);

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::struct_write_le<test_field_list>(stream, obj);
  ASSERT_FALSE(written.has_value());
  EXPECT_EQ(written.error().failure_reason, s2s::error_reason::validation_failure);
  EXPECT_EQ(written.error().failed_at, "body");
}

TEST(WriteUnionFields, RoundTripsComputedSwitchUnionsWhenTheyAgree) {
  using test_field_list = computed_schema;

  computed_schema obj{};
  obj["a"_f] = 50;
  obj["b"_f] = 50;
  obj["body"_f] = static_cast<u32>(0xcafed00d);

  FIELD_LIST_LE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::get<u32>((*result)["body"_f]), 0xcafed00d);
  });
}

// No branch matches, so deduce_type itself fails and the reason propagates
// unchanged rather than being reported as a validation failure.
TEST(WriteUnionFields, PropagatesTypeDeductionFailure) {
  auto never = [](auto a, auto b) { return a + b == 0xffffffff; };
  using test_field_list =
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::variance<
        "body",
        s2s::type<
          s2s::type_if_else<
            s2s::branch<
              s2s::predicate<never, s2s::with_fields<"a", "b">>,
              s2s::as_trivial<u32, s2s::field_size<s2s::fixed<4>>>
            >
          >
        >
      >
    >;

  test_field_list obj{};
  obj["a"_f] = 1;
  obj["b"_f] = 1;
  obj["body"_f] = static_cast<u32>(7);

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::struct_write_le<test_field_list>(stream, obj);
  ASSERT_FALSE(written.has_value());
  EXPECT_EQ(written.error().failure_reason, s2s::error_reason::type_deduction_failure);
  EXPECT_EQ(written.error().failed_at, "body");
}

// Fields feeding ladder predicates are ordinary data — nothing about them is
// invertible, so 005's constraint must leave them assignable.
TEST(WriteUnionFields, FieldsFeedingLadderPredicatesStayAssignable) {
  ladder_schema obj{};
  obj["a"_f] = 3;
  obj["b"_f] = 4;
  EXPECT_EQ(obj["a"_f], 3u);
  EXPECT_EQ(obj["b"_f], 4u);
}

// A length whose only producer is a union alternative is conditional: it is
// verified while that alternative is held, and left alone otherwise.
TEST(WriteUnionFields, VerifiesALengthObligatedByAUnionAlternative) {
  using test_field_list =
    s2s::struct_field_list<
      s2s::basic_field<"tag", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::variance<
        "body",
        s2s::type<
          s2s::match_field<"tag">,
          s2s::type_switch<
            s2s::match_case<
              0xcafed00d,
              s2s::as_vec<u8, s2s::field_size<s2s::len_from_field<"len">>>
            >,
            s2s::match_case<
              0xdeadbeef,
              s2s::as_trivial<u32, s2s::field_size<s2s::fixed<4>>>
            >
          >
        >
      >
    >;

  test_field_list agreeing{};
  agreeing["len"_f] = 3;
  agreeing["body"_f] = std::vector<u8>{1, 2, 3};

  FIELD_LIST_BE_ROUNDTRIP_CHECK(agreeing, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::get<std::vector<u8>>((*result)["body"_f]), (std::vector<u8>{1, 2, 3}));
  });

  test_field_list disagreeing{};
  disagreeing["len"_f] = 9;
  disagreeing["body"_f] = std::vector<u8>{1, 2, 3};

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::struct_write_le<test_field_list>(stream, disagreeing);
  ASSERT_FALSE(written.has_value());
  EXPECT_EQ(written.error().failure_reason, s2s::error_reason::found_contradicting_length);
  EXPECT_EQ(written.error().failed_at, "len");

  // The other alternative does not obligate "len" at all, so it goes out as
  // stored.
  test_field_list other{};
  other["len"_f] = 9;
  other["body"_f] = static_cast<u32>(0x11223344);

  std::stringstream untouched(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::struct_write_be<test_field_list>(untouched, other).has_value());
  EXPECT_EQ(untouched.str(), std::string("\xde\xad\xbe\xef\x00\x00\x00\x09\x11\x22\x33\x44", 12));
}
