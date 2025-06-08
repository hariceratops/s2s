#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

TEST(FieldConstraintTest, EqConstraint) {
  auto eq_obj = s2s::eq(42);
  EXPECT_TRUE(eq_obj(42));
  EXPECT_FALSE(eq_obj(84));
}

TEST(FieldConstraintTest, NeqConstraint) {
  auto neq_obj = s2s::neq(42);
  EXPECT_FALSE(neq_obj(42));
  EXPECT_TRUE(neq_obj(84));
}

TEST(FieldConstraintTest, LtConstraint) {
  auto lt_obj = s2s::lt(42);
  EXPECT_TRUE(lt_obj(21));
  EXPECT_FALSE(lt_obj(84));
}

TEST(FieldConstraintTest, GtConstraint) {
  auto gt_obj = s2s::gt(42);
  EXPECT_FALSE(gt_obj(21));
  EXPECT_TRUE(gt_obj(84));
}

TEST(FieldConstraintTest, LteConstraint) {
  auto lte_obj = s2s::lte(42);
  EXPECT_TRUE(lte_obj(21));
  EXPECT_TRUE(lte_obj(42));
  EXPECT_FALSE(lte_obj(84));
}

TEST(FieldConstraintTest, GteConstraint) {
  auto gte_obj = s2s::gte(42);
  EXPECT_FALSE(gte_obj(21));
  EXPECT_TRUE(gte_obj(42));
  EXPECT_TRUE(gte_obj(84));
}

TEST(FieldConstraintTest, AnyOfConstraint) {
  auto any_of_obj = s2s::any_of(21, 42, 84);
  EXPECT_TRUE(any_of_obj(21));
  EXPECT_TRUE(any_of_obj(42));
  EXPECT_TRUE(any_of_obj(84));
  EXPECT_FALSE(any_of_obj(100));
}

// TEST(FieldConstraintTest, OpenRangeIntervalCheck) {
//   // constexpr auto ranges = std::array<range<u32>, 3>{range{2u, 4u}, range{6u, 9u}, range{13u, 15u}};
//   // constexpr auto r = range{2u, 4u};
// }

