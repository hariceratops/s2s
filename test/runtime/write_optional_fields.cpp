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

// An optional buffer must not copy its payload to write it.
TEST(WriteOptionalFields, RoundTripsAnOptionalVariableSizedField) {
  using test_field_list =
    s2s::struct_field_list<
      s2s::basic_field<"flag", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::maybe<
        s2s::vec_field<"data", u16, s2s::field_size<s2s::len_from_field<"len">>>,
        s2s::parse_if<flag_is_set, s2s::with_fields<"flag">>
      >
    >;

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
