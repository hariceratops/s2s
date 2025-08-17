#ifndef _TYPE_DEDUCTION_SWITCH_TRAITS_HPP_
#define _TYPE_DEDUCTION_SWITCH_TRAITS_HPP_

#include "type_deduction_match_case_traits.hpp"
#include "type_deduction_switch.hpp"


namespace s2s {
template <typename T>
struct is_type_switch;

template <typename T>
struct is_type_switch {
  static constexpr bool res = false;
};

template <match_case_like case_head, match_case_like... case_tail>
struct is_type_switch<type_switch<case_head, case_tail...>> {
  static constexpr bool res = true;
};

template <typename T>
static constexpr bool is_type_switch_v = is_type_switch<T>::res;

template <typename T>
concept type_switch_like = is_type_switch_v<T>;

} /* namespace s2s */


#endif // _TYPE_DEDUCTION_SWITCH_TRAITS_HPP_
