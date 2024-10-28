#ifndef _TYPE_LADDER_HPP_
#define _TYPE_LADDER_HPP_

#include "compute_res.hpp"
#include <expected>


// todo return type tag constructed from clause
template <typename... clauses>
struct type_ladder;

template <std::size_t idx, typename... clauses>
struct type_ladder_impl;

template <std::size_t idx>
struct type_ladder_impl<idx> {
  constexpr auto operator()(auto) const -> 
    std::expected<std::size_t, std::string> 
  {
    return std::unexpected("no matches found");
  }
};

// todo constrain clause head and clause_rest
template <std::size_t idx, typename clause_head, typename... clause_rest>
struct type_ladder_impl<idx, clause_head, clause_rest...> {
  template <typename... fields>
  constexpr auto operator()(struct_field_list<fields...>& field_list) const -> 
    std::expected<std::size_t, std::string> 
  {
    bool eval_result = clause_head{}(field_list);
    if(eval_result) return idx;
    else return type_ladder_impl<idx + 1, clause_rest...>{}(field_list);
  }
};

template <typename clause_head, typename... clause_rest>
struct type_ladder<clause_head, clause_rest...> {
  // ? is this ok
  using types_only = typelist::typelist<typename clause_head::type_tag::type, typename clause_rest::type_tag::type...>;
  using size_only = std::integer_sequence<std::size_t, clause_head::type_tag::size, clause_rest::type_tag::size...>;

  template <typename... fields>
  constexpr auto operator()(struct_field_list<fields...>& field_list) const -> 
    std::expected<std::size_t, std::string> 
  {
    return type_ladder_impl<0, clause_head, clause_rest...>{}(field_list);
  }
};


#endif // _TYPE_LADDER_HPP_
