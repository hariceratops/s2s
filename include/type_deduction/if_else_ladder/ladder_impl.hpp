#ifndef _LADDER_IMPL_HPP_
#define _LADDER_IMPL_HPP_

#include <expected>
#include "../../error/cast_error.hpp"
#include "../../field_list/field_list.hpp"
#include "../../field_compute/computation_from_fields_impl.hpp"
#include "ladder.hpp"
#include "../utils/helper.hpp"


namespace s2s {
template <typename ladder>
struct evaluate_ladder;

template <std::size_t idx, typename branch>
struct evalaute_ladder_impl;

template <std::size_t idx, typename branch>
struct evalaute_ladder_impl {
  template <auto metadata, typename... fields>
  constexpr auto operator()(const struct_field_list_impl<metadata, fields...>& field_list) const -> 
    type_deduction_idx
  {
    if(compute_impl<typename branch::expression>{}(field_list)) 
      return idx;
    return std::nullopt;
  }
};

template <typename... branches>
struct evaluate_ladder_helper {
  template <auto metadata, typename... fields, std::size_t... idx>
  constexpr auto operator()(
    const struct_field_list_impl<metadata, fields...>& field_list, 
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
  template <auto metadata, typename... fields>
  constexpr auto operator()(const struct_field_list_impl<metadata, fields...>& field_list) const -> 
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

#endif // _LADDER_IMPL_HPP_
