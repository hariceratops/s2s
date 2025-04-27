#ifndef _TYPE_DEDUCTION_SWITCH_HPP_
#define _TYPE_DEDUCTION_SWITCH_HPP_

#include <expected>
#include "cast_error.hpp"
#include "type_deduction_match_case.hpp"
#include "type_deduction_helper.hpp"


namespace s2s {
template <std::size_t idx, typename... cases>
struct type_switch_impl;

template <std::size_t idx>
struct type_switch_impl<idx> {
  constexpr auto operator()(const auto&) const -> 
    std::expected<std::size_t, error_reason> 
  {
    return std::unexpected(error_reason::type_deduction_failure);
  }
};

// todo check if case and eval result match in terms of types
template <std::size_t idx, match_case_like match_case_head, match_case_like... match_case_rest>
struct type_switch_impl<idx, match_case_head, match_case_rest...> {
  constexpr auto operator()(const auto& v) const -> 
    std::expected<std::size_t, error_reason> 
  {
    if(v == match_case_head::value) return idx;
    else return type_switch_impl<idx + 1, match_case_rest...>{}(v);
  }
};

// atleast one type has to match? but anyways if nothing is matches we get 
// std::unexpected
// todo constrain eval to compute type, cases to match cases
// todo constrain eval return type matches all match case values
// todo return tag constructed with match
template <match_case_like case_head, match_case_like... case_rest>
struct type_switch {
  using variant = variant_from_type_conditions_v<case_head, case_rest...>;
  using sizes = size_choices_from_type_conditions_v<case_head, case_rest...>;

  template <typename... fields>
  constexpr auto operator()(const auto& v) const -> 
    std::expected<std::size_t, error_reason> 
  {
    return type_switch_impl<0, case_head, case_rest...>{}(v);
  } 
};

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


#endif // _TYPE_SWITCH_HPP_
