#ifndef _TYPE_DEDUCTION_LADDER_IMPL_HPP_
#define _TYPE_DEDUCTION_LADDER_IMPL_HPP_

#include "../error/cast_error.hpp"
#include "type_deduction_ladder.hpp"
#include "type_deduction_helper.hpp"
#include <expected>


namespace s2s {
template <typename ladder>
struct evaluate_ladder;

template <std::size_t idx, typename branch>
struct evalaute_ladder_impl;

template <std::size_t idx, typename branch>
struct evalaute_ladder_impl {
  template <typename... fields>
  constexpr auto operator()(const struct_field_list_impl<fields...>& field_list) const -> 
    type_deduction_idx
  {
    if(branch::e(field_list)) return idx;
    return std::nullopt;
  }
};

template <typename... branches>
struct evaluate_ladder_helper {
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
      [&]() { return evalaute_ladder_impl<idx, branches>{}(field_list); }
    );
  }
};


template <typename... branches>
struct evaluate_ladder<type_if_else<branches...>> {
  template <typename... fields>
  constexpr auto operator()(const struct_field_list_impl<fields...>& field_list) const -> 
    type_deduction_res
  {
    auto res = evaluate_ladder_helper<branches...>{}(
      field_list,
      std::make_index_sequence<sizeof...(branches)>{}
    );
    if(!res)
      return std::unexpected(error_reason::type_deduction_failure);
    return std::expected<std::size_t, error_reason>(*res);
  }
};
} /* namespace s2s */

#endif // _TYPE_DEDUCTION_LADDER_IMPL_HPP_
