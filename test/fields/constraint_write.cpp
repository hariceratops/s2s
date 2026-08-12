#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// Relocated from test/runtime/write_magic_fields.cpp, whose own comment says
// this is not a magic-field behaviour. 036 fills this out.
TEST(ConstraintWrite, RejectsConstraintViolationOnAnOrdinaryField) {
  using test_field_list =
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::basic_field<"bounded", u32, s2s::field_size<s2s::fixed<4>>, s2s::lt{100u}>
    >;

  test_field_list obj{};
  obj["a"_f] = 0xdeadbeef;
  obj["bounded"_f] = 500;

  FIELD_LIST_LE_WRITE_REJECTED(obj, s2s::error_reason::validation_failure);
}
