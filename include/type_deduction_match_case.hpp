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

#endif // _TYPE_DEDUCTION_MATCH_CASE_HPP_
