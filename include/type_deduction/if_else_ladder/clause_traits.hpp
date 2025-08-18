#ifndef _CLAUSE_TRAITS_HPP_
#define _CLAUSE_TRAITS_HPP_

#include "clause.hpp"

namespace s2s {
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

#endif // _CLAUSE_TRAITS_HPP_
