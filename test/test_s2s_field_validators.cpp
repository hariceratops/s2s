#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include "../single_header/struct_cast.hpp"
#include "s2s_test_utils.hpp"


using namespace s2s_literals;


TEST_CASE("Test eq field constraint") {
  auto eq_obj = s2s::eq(42);
  REQUIRE(eq_obj(42) == true);
  REQUIRE(eq_obj(84) == false);
}


TEST_CASE("Test neq field constraint") {
  auto neq_obj = s2s::neq(42);
  REQUIRE(neq_obj(42) == false);
  REQUIRE(neq_obj(84) == true);
}


TEST_CASE("Test lt field constraint") {
  auto lt_obj = s2s::lt(42);
  REQUIRE(lt_obj(21) == true);
  REQUIRE(lt_obj(84) == false);
}


TEST_CASE("Test gt field constraint") {
  auto gt_obj = s2s::gt(42);
  REQUIRE(gt_obj(21) == false);
  REQUIRE(gt_obj(84) == true);
}


TEST_CASE("Test lte field constraint") {
  auto lte_obj = s2s::lte(42);
  REQUIRE(lte_obj(21) == true);
  REQUIRE(lte_obj(42) == true);
  REQUIRE(lte_obj(84) == false);
}


TEST_CASE("Test gte field constraint") {
  auto gte_obj = s2s::gte(42);
  REQUIRE(gte_obj(21) == false);
  REQUIRE(gte_obj(42) == true);
  REQUIRE(gte_obj(84) == true);
}


TEST_CASE("Test any_of field constraint") {
  auto any_of_obj = s2s::any_of(21, 42, 84);
  REQUIRE(any_of_obj(21) == true);
  REQUIRE(any_of_obj(42) == true);
  REQUIRE(any_of_obj(84) == true);
  REQUIRE(any_of_obj(100) == false);
}


// TEST_CASE("Test open range interval check") {
//   // constexpr auto ranges = std::array<range<u32>, 3>{range{2u, 4u}, range{6u, 9u}, range{13u, 15u}};
//   // constexpr auto r = range{2u, 4u};
// }

