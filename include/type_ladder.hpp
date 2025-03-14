#ifndef _TYPE_LADDER_HPP_
#define _TYPE_LADDER_HPP_

#include "error.hpp"
#include "type_deduction_helper.hpp"
#include <expected>


// todo return type tag constructed from clause
template <typename... clauses>
struct type_ladder;

template <std::size_t idx, typename... clauses>
struct type_ladder_impl;

template <std::size_t idx>
struct type_ladder_impl<idx> {
  constexpr auto operator()(const auto&) const -> 
    std::expected<std::size_t, cast_error> 
  {
    return std::unexpected(cast_error::type_deduction_failure);
  }
};

// todo constrain clause head and clause_rest
template <std::size_t idx, typename clause_head, typename... clause_rest>
struct type_ladder_impl<idx, clause_head, clause_rest...> {
  template <typename... fields>
  constexpr auto operator()(const struct_field_list<fields...>& field_list) const -> 
    std::expected<std::size_t, cast_error> 
  {
    bool eval_result = clause_head::e(field_list);
    if(eval_result) return idx;
    else return type_ladder_impl<idx + 1, clause_rest...>{}(field_list);
  }
};

template <typename clause_head, typename... clause_rest>
struct type_ladder<clause_head, clause_rest...> {
  // ? is this ok
  using variant = variant_from_type_conditions_v<clause_head, clause_rest...>;
  using sizes = size_choices_from_type_conditions_v<clause_head, clause_rest...>;

  template <typename... fields>
  constexpr auto operator()(const struct_field_list<fields...>& field_list) const -> 
    std::expected<std::size_t, cast_error> 
  {
    return type_ladder_impl<0, clause_head, clause_rest...>{}(field_list);
  }
};


#endif // _TYPE_LADDER_HPP_
