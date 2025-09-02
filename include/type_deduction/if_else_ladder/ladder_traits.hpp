#ifndef _LADDER_TRAITS_HPP_
#define _LADDER_TRAITS_HPP_


#include "ladder.hpp"


namespace s2s {
template <typename T>
struct is_type_if_else;

template <typename T>
struct is_type_if_else {
  static constexpr bool res = false;
};

template <branch_like branch_head, branch_like... branch_tail>
struct is_type_if_else<type_if_else<branch_head, branch_tail...>> {
  static constexpr bool res = true;
};

template <typename T>
static constexpr bool is_type_if_else_v = is_type_if_else<T>::res;

template <typename T>
concept type_if_else_like = is_type_if_else_v<T>;
} /* namespace s2s */

#endif // _LADDER_TRAITS_HPP_
