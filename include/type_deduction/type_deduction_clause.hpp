#ifndef _TYPE_DEDUCTION_CLAUSE_HPP_
#define _TYPE_DEDUCTION_CLAUSE_HPP_

#include "type_deduction_tags.hpp"
#include "../field_compute/computation_from_fields.hpp"


namespace s2s {

template <typename T>
concept evaluates_to_bool = is_eval_bool_from_fields_v<T>;

template <evaluates_to_bool eval, type_tag_like T>
struct branch {
  static constexpr auto e = eval{};
  using type_tag = T;
};


template <typename T>
struct is_branch;

template <typename T>
struct is_branch {
  static constexpr bool res = false;
};

template <typename eval, typename T>
struct is_branch<branch<eval, T>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_branch_v = is_branch<T>::res;

template <typename T>
concept branch_like = is_branch_v<T>;
} /* namespace s2s */

#endif // _TYPE_DEDUCTION_CLAUSE_HPP_
