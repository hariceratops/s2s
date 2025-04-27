#ifndef _TYPE_DEDUCTION_CLAUSE_HPP_
#define _TYPE_DEDUCTION_CLAUSE_HPP_

#include "type_deduction_tags.hpp"
#include "computation_from_fields.hpp"


namespace s2s {
template <typename eval, type_tag_like T>
  requires is_eval_bool_from_fields_v<eval>
struct clause {
  static constexpr auto e = eval{};
  using type_tag = T;
};


template <typename T>
struct is_clause;

template <typename T>
struct is_clause {
  static constexpr bool res = false;
};

template <typename eval, typename T>
struct is_clause<clause<eval, T>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_clause_v = is_clause<T>::res;

template <typename T>
concept clause_like = is_clause_v<T>;
} /* namespace s2s */

#endif // _TYPE_DEDUCTION_CLAUSE_HPP_
