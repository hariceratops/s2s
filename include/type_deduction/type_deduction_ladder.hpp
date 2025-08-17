#ifndef _TYPE_DEDUCTION_LADDER_HPP_
#define _TYPE_DEDUCTION_LADDER_HPP_

#include "../error/cast_error.hpp"
#include "type_deduction_clause.hpp"
#include "type_deduction_clause_traits.hpp"
#include "type_deduction_helper.hpp"
#include <expected>


namespace s2s {
template <branch_like... branches>
  requires (sizeof...(branches) > 0)
struct type_if_else;

template <std::size_t idx, typename branch>
struct type_if_else_impl;

template <std::size_t idx, typename branch>
struct type_if_else_impl {
  template <typename... fields>
  constexpr auto operator()(const struct_field_list_impl<fields...>& field_list) const -> 
    type_deduction_idx
  {
    if(branch::e(field_list)) return idx;
    return std::nullopt;
  }
};

template <typename... branches>
struct type_if_else_helper {
  template <typename... fields, std::size_t... idx>
  constexpr auto operator()(
    const struct_field_list_impl<fields...>& field_list, 
    const std::index_sequence<idx...>&) const 
  -> type_deduction_idx
  {
    type_deduction_idx pipeline_seed = std::nullopt;
    return (
      pipeline_seed |
      ... |
      [&]() { return type_if_else_impl<idx, branches>{}(field_list); }
    );
  }
};


template <branch_like... branches>
  requires (sizeof...(branches) > 0)
struct type_if_else {
  using variant = variant_from_type_conditions_v<branches...>;
  using sizes = size_choices_from_type_conditions_v<branches...>;

  template <typename... fields>
  constexpr auto operator()(const struct_field_list_impl<fields...>& field_list) const -> 
    type_deduction_res
  {
    auto res = type_if_else_helper<branches...>{}(
      field_list,
      std::make_index_sequence<sizeof...(branches)>{}
    );
    if(!res)
      return std::unexpected(error_reason::type_deduction_failure);
    return std::expected<std::size_t, error_reason>(*res);
  }
};
} /* namespace s2s */

#endif // _TYPE_LADDER_HPP_
