#ifndef _MATCH_CASE_TRAITS_HPP_
#define _MATCH_CASE_TRAITS_HPP_

#include "match_case.hpp"


namespace s2s {
template <typename T>
struct is_match_case;

template <auto v, typename h>
struct is_match_case<match_case<v, h>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_match_case {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_match_case_v = is_match_case<T>::res;

template <typename T>
concept match_case_like = is_match_case_v<T>;
} /* namespace s2s */

#endif // _MATCH_CASE_TRAITS_HPP_
