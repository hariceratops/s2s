#ifndef _TYPE_DEDUCTION_CLAUSE_HPP_
#define _TYPE_DEDUCTION_CLAUSE_HPP_

#include "type_deduction_tags.hpp"
#include "../field_compute/computation_from_fields_traits.hpp"


namespace s2s {

template <typename T>
concept evaluates_to_bool = is_eval_bool_from_fields_v<T>;

template <evaluates_to_bool eval, type_tag_like T>
struct branch {
  using expression = eval;
  using type_tag = T;
};
} /* namespace s2s */

#endif // _TYPE_DEDUCTION_CLAUSE_HPP_
