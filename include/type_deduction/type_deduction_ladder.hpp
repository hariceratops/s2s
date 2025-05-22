#ifndef _TYPE_DEDUCTION_LADDER_HPP_
#define _TYPE_DEDUCTION_LADDER_HPP_

#include "../error/cast_error.hpp"
#include "type_deduction_helper.hpp"
#include <expected>


namespace s2s {
template <typename... branches>
struct type_if_else;

template <std::size_t idx, typename... branches>
struct type_if_else_impl;

template <std::size_t idx>
struct type_if_else_impl<idx> {
  constexpr auto operator()(const auto&) const -> 
    std::expected<std::size_t, error_reason> 
  {
    return std::unexpected(error_reason::type_deduction_failure);
  }
};

template <std::size_t idx, typename branch_head, typename... branch_rest>
struct type_if_else_impl<idx, branch_head, branch_rest...> {
  template <typename... fields>
  constexpr auto operator()(const struct_field_list_impl<fields...>& field_list) const -> 
    std::expected<std::size_t, error_reason> 
  {
    bool eval_result = branch_head::e(field_list);
    if(eval_result) return idx;
    else return type_if_else_impl<idx + 1, branch_rest...>{}(field_list);
  }
};

template <branch_like branch_head, branch_like... branch_rest>
struct type_if_else<branch_head, branch_rest...> {
  // ? is this ok
  using variant = variant_from_type_conditions_v<branch_head, branch_rest...>;
  using sizes = size_choices_from_type_conditions_v<branch_head, branch_rest...>;

  template <typename... fields>
  constexpr auto operator()(const struct_field_list_impl<fields...>& field_list) const -> 
    std::expected<std::size_t, error_reason> 
  {
    return type_if_else_impl<0, branch_head, branch_rest...>{}(field_list);
  }
};

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

#endif // _TYPE_LADDER_HPP_
