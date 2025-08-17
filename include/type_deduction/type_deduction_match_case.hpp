#ifndef _TYPE_DEDUCTION_MATCH_CASE_HPP_
#define _TYPE_DEDUCTION_MATCH_CASE_HPP_

#include "type_deduction_tags.hpp"


namespace s2s {
// todo constrain to data types possible for fields
template <auto v, type_tag_like T>
struct match_case {
  static constexpr auto value = v;
  using type_tag = T;
};
} /* namespace s2s */

#endif // _TYPE_DEDUCTION_MATCH_CASE_HPP_
