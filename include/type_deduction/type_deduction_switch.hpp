#ifndef _TYPE_DEDUCTION_SWITCH_HPP_
#define _TYPE_DEDUCTION_SWITCH_HPP_

#include <expected>
#include "../error/cast_error.hpp"
#include "type_deduction_match_case.hpp"
#include "type_deduction_match_case_traits.hpp"
#include "type_deduction_helper.hpp"


namespace s2s {

// todo check if case and eval result match in terms of types
template <std::size_t idx, typename match_case>
struct type_switch_impl {
  constexpr auto operator()(const auto& v) const -> 
    type_deduction_idx
  {
    if(v == match_case::value) return idx;
    else return std::nullopt;
  }
};

template <typename... branches>
struct type_switch_helper {
  template <std::size_t... idx>
  constexpr auto operator()(
    const auto& v, 
    const std::index_sequence<idx...>&) const 
  -> type_deduction_idx 
  {
    type_deduction_idx pipeline_seed = std::nullopt;
    return (
      pipeline_seed |
      ... |
      [&]() { return type_switch_impl<idx, branches>{}(v); }
    );
  }
};

template <match_case_like... cases>
  requires (sizeof...(cases) > 0)
struct type_switch {
  using variant = variant_from_type_conditions_v<cases...>;
  using sizes = size_choices_from_type_conditions_v<cases...>;

  constexpr auto operator()(const auto& v) const -> 
    type_deduction_res
  {
    auto res =
      type_switch_helper<cases...>{}(
        v, std::make_index_sequence<sizeof...(cases)>{}
      );
    if(!res)
      return std::unexpected(error_reason::type_deduction_failure);
    return std::expected<std::size_t, error_reason>(*res);
  } 
};
} /* namespace s2s */


#endif // _TYPE_SWITCH_HPP_
