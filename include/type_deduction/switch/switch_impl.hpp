#ifndef _SWITCH_IMPL_HPP_
#define _SWITCH_IMPL_HPP_

#include <expected>
#include "../../error/cast_error.hpp"
#include "switch.hpp"
#include "../utils/helper.hpp"


namespace s2s {

// todo check if case and eval result match in terms of types
template <std::size_t idx, typename match_case>
struct evaluate_switch_impl {
  constexpr auto operator()(const auto& v) const -> type_deduction_idx
  {
    if(v == match_case::value) return idx;
    else return std::nullopt;
  }
};

template <typename... match_cases>
struct evaluate_switch_helper {
  template <std::size_t... idx>
  constexpr auto operator()(const auto& v, const std::index_sequence<idx...>&) const 
  -> type_deduction_idx 
  {
    type_deduction_idx pipeline_seed = std::nullopt;
    return (
      pipeline_seed |
      ... |
      [&]() { return evaluate_switch_impl<idx, match_cases>{}(v); }
    );
  }
};

template <typename _switch>
struct evaluate_switch;

template <typename... cases>
struct evaluate_switch<type_switch<cases...>> {
  constexpr auto operator()(const auto& v) const -> type_deduction_res
  {
    auto res =
      evaluate_switch_helper<cases...>{}(
        v, std::make_index_sequence<sizeof...(cases)>{}
      );
    if(!res)
      return std::unexpected(error_reason::type_deduction_failure);
    return std::expected<std::size_t, error_reason>(*res);
  } 
};
} /* namespace s2s */


#endif // _SWITCH_IMPL_HPP_
