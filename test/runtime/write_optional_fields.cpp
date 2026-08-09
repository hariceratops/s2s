#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

namespace {
auto flag_is_set = [](auto flag) { return flag == 0xdeadbeef; };

using optional_schema =
  s2s::struct_field_list<
    s2s::basic_field<"flag", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::maybe<
      s2s::basic_field<"payload", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::parse_if<flag_is_set, s2s::with_fields<"flag">>
    >
  >;
} /* namespace */

TEST(WriteOptionalFields, RoundTripsWhenPresentInBothByteOrders) {
  using test_field_list = optional_schema;

  optional_schema obj{};
  obj["flag"_f] = 0xdeadbeef;
  obj["payload"_f] = 0xcafed00d;

  FIELD_LIST_LE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE((*result)["payload"_f].has_value());
    EXPECT_EQ(*(*result)["payload"_f], 0xcafed00d);
  });

  FIELD_LIST_BE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE((*result)["payload"_f].has_value());
    EXPECT_EQ(*(*result)["payload"_f], 0xcafed00d);
  });
}

TEST(WriteOptionalFields, EmitsNothingWhenAbsent) {
  using test_field_list = optional_schema;

  optional_schema obj{};
  obj["flag"_f] = 0x11223344;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::struct_write_le<test_field_list>(stream, obj).has_value());
  EXPECT_EQ(stream.str(), std::string("\x44\x33\x22\x11", 4));

  FIELD_LIST_LE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE((*result)["payload"_f].has_value());
  });
}

// The reader would go looking for four bytes that were never written.
TEST(WriteOptionalFields, RejectsPredicateTrueButOptionalEmpty) {
  using test_field_list = optional_schema;

  optional_schema obj{};
  obj["flag"_f] = 0xdeadbeef;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::struct_write_le<test_field_list>(stream, obj);
  ASSERT_FALSE(written.has_value());
  EXPECT_EQ(written.error().failure_reason, s2s::error_reason::validation_failure);
  EXPECT_EQ(written.error().failed_at, "payload");
  EXPECT_EQ(stream.str().size(), 4u);
}

// Emitting nothing here would silently discard data the caller handed in.
TEST(WriteOptionalFields, RejectsPredicateFalseButOptionalEngaged) {
  using test_field_list = optional_schema;

  optional_schema obj{};
  obj["flag"_f] = 0x11223344;
  obj["payload"_f] = 0xcafed00d;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::struct_write_le<test_field_list>(stream, obj);
  ASSERT_FALSE(written.has_value());
  EXPECT_EQ(written.error().failure_reason, s2s::error_reason::validation_failure);
  EXPECT_EQ(written.error().failed_at, "payload");
  EXPECT_EQ(stream.str().size(), 4u);
}

TEST(WriteOptionalFields, RoundTripsAlwaysPresentWithoutSpecialCasing) {
  using test_field_list =
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::maybe<
        s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
        s2s::always_present
      >
    >;

  test_field_list obj{};
  obj["a"_f] = 0x11223344;
  obj["b"_f] = 0xcafed00d;

  FIELD_LIST_LE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE((*result)["b"_f].has_value());
    EXPECT_EQ(*(*result)["b"_f], 0xcafed00d);
  });
}

// Presence is a predicate, not a stored flag, so there is nothing to derive
// and 005's constraint must leave the siblings that feed it alone.
TEST(WriteOptionalFields, SiblingsFeedingThePredicateStayAssignable) {
  optional_schema obj{};
  obj["flag"_f] = 0x1;
  EXPECT_EQ(obj["flag"_f], 0x1u);

  static_assert(
    !s2s::is_derived_field<meta::type_id<s2s::field_list_metadata<
      s2s::basic_field<"flag", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::maybe<
        s2s::basic_field<"payload", u32, s2s::field_size<s2s::fixed<4>>>,
        s2s::parse_if<flag_is_set, s2s::with_fields<"flag">>
      >
    >>>("flag"));
}

namespace {
// "len" sizes a vector that lives inside a maybe, so it is a *conditional*
// source: the optional may be absent, and then there is nothing to derive
// from. It stays writable and is verified only when the optional is present.
using conditional_len_schema =
  s2s::struct_field_list<
    s2s::basic_field<"flag", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::maybe<
      s2s::vec_field<"data", u16, s2s::field_size<s2s::len_from_field<"len">>>,
      s2s::parse_if<flag_is_set, s2s::with_fields<"flag">>
    >
  >;
} /* namespace */

// An optional buffer must not copy its payload to write it.
TEST(WriteOptionalFields, RoundTripsAnOptionalVariableSizedField) {
  using test_field_list = conditional_len_schema;

  test_field_list obj{};
  obj["flag"_f] = 0xdeadbeef;
  obj["len"_f] = 3;
  obj["data"_f] = std::vector<u16>{0x1122, 0x3344, 0x5566};

  FIELD_LIST_BE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE((*result)["data"_f].has_value());
    EXPECT_EQ(*(*result)["data"_f], (std::vector<u16>{0x1122, 0x3344, 0x5566}));
  });
}

// A conditional source is not derived, so a wrong length is the caller's to
// get wrong — and must be caught rather than written out.
TEST(WriteOptionalFields, RejectsAConditionalLengthThatDisagreesWhenPresent) {
  using test_field_list = conditional_len_schema;

  test_field_list obj{};
  obj["flag"_f] = 0xdeadbeef;
  obj["len"_f] = 7;
  obj["data"_f] = std::vector<u16>{0x1122, 0x3344, 0x5566};

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::struct_write_le<test_field_list>(stream, obj);
  ASSERT_FALSE(written.has_value());
  EXPECT_EQ(written.error().failure_reason, s2s::error_reason::found_contradicting_length);
  EXPECT_EQ(written.error().failed_at, "len");
  // Only the flag is out; the contradicting length never reached the stream.
  EXPECT_EQ(stream.str().size(), 4u);
}

// When the optional is absent there is no obligation at all, so the length
// keeps whatever the caller stored — the reader will not consult it either.
TEST(WriteOptionalFields, LeavesAConditionalLengthAloneWhenAbsent) {
  using test_field_list = conditional_len_schema;

  test_field_list obj{};
  obj["flag"_f] = 0x11223344;
  obj["len"_f] = 7;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::struct_write_be<test_field_list>(stream, obj).has_value());
  EXPECT_EQ(stream.str(), std::string("\x11\x22\x33\x44\x00\x00\x00\x07", 8));
}

namespace {
// "len" has an unconditional producer (the top-level vector) *and* a
// conditional one (the vector inside the maybe). It is derived from the
// former, and the latter is verified against that derived value when present.
using mixed_len_schema =
  s2s::struct_field_list<
    s2s::basic_field<"flag", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::vec_field<"always", u8, s2s::field_size<s2s::len_from_field<"len">>>,
    s2s::maybe<
      s2s::vec_field<"sometimes", u8, s2s::field_size<s2s::len_from_field<"len">>>,
      s2s::parse_if<flag_is_set, s2s::with_fields<"flag">>
    >
  >;
} /* namespace */

TEST(WriteOptionalFields, DerivesFromTheUnconditionalProducerAndVerifiesTheConditionalOne) {
  using test_field_list = mixed_len_schema;

  test_field_list obj{};
  obj["flag"_f] = 0xdeadbeef;
  obj["always"_f] = std::vector<u8>{1, 2, 3};
  obj["sometimes"_f] = std::vector<u8>{4, 5, 6};

  FIELD_LIST_BE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["len"_f], 3u);
    EXPECT_EQ((*result)["always"_f], (std::vector<u8>{1, 2, 3}));
    ASSERT_TRUE((*result)["sometimes"_f].has_value());
    EXPECT_EQ(*(*result)["sometimes"_f], (std::vector<u8>{4, 5, 6}));
  });
}

// Without the verify-when-active path this would derive 3 from "always" and
// then write a 2-element "sometimes" behind it, producing a stream that does
// not read back.
TEST(WriteOptionalFields, RejectsAConditionalProducerThatDisagreesWithTheDerivedValue) {
  using test_field_list = mixed_len_schema;

  test_field_list obj{};
  obj["flag"_f] = 0xdeadbeef;
  obj["always"_f] = std::vector<u8>{1, 2, 3};
  obj["sometimes"_f] = std::vector<u8>{4, 5};

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  auto written = s2s::struct_write_le<test_field_list>(stream, obj);
  ASSERT_FALSE(written.has_value());
  EXPECT_EQ(written.error().failure_reason, s2s::error_reason::found_contradicting_length);
  EXPECT_EQ(written.error().failed_at, "len");
  EXPECT_EQ(stream.str().size(), 4u);
}

// The same disagreement is not an error when the optional is absent, because
// then it contributes no bytes for the length to describe.
TEST(WriteOptionalFields, IgnoresAnAbsentConditionalProducerWhenDeriving) {
  using test_field_list = mixed_len_schema;

  test_field_list obj{};
  obj["flag"_f] = 0x11223344;
  obj["always"_f] = std::vector<u8>{1, 2, 3};

  FIELD_LIST_BE_ROUNDTRIP_CHECK(obj, {
    ASSERT_TRUE(written.has_value());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["len"_f], 3u);
    EXPECT_FALSE((*result)["sometimes"_f].has_value());
  });
}

// Conditional sources are not derived, so 005's constraint must not reach
// them: the caller has to be able to set the length.
TEST(WriteOptionalFields, AConditionalLengthStaysAssignable) {
  conditional_len_schema obj{};
  obj["len"_f] = 5;
  EXPECT_EQ(obj["len"_f], 5u);

  static_assert(
    !s2s::is_derived_field<meta::type_id<s2s::field_list_metadata<
      s2s::basic_field<"flag", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::maybe<
        s2s::vec_field<"data", u16, s2s::field_size<s2s::len_from_field<"len">>>,
        s2s::parse_if<flag_is_set, s2s::with_fields<"flag">>
      >
    >>>("len"));
}
