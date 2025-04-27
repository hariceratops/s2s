#ifndef _FIELD_VALE_CONSTRAINT_TRAITS_HPP_
#define _FIELD_VALE_CONSTRAINT_TRAITS_HPP_

#include "field_value_constraints.hpp"

namespace s2s {
template <typename T>
struct is_no_constraint;

template <typename T>
struct is_no_constraint<no_constraint<T>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_no_constraint {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_no_constraint_v = is_no_constraint<T>::res;
}


#endif /* _FIELD_VALE_CONSTRAINT_TRAITS_HPP_ */
