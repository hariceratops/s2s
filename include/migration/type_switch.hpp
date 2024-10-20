#ifndef _TYPE_SWITCH_HPP_
#define _TYPE_SWITCH_HPP_

#include "compute_res.hpp"
#include "match_case.hpp"


template <std::size_t idx, typename... cases>
struct type_switch_impl;

template <std::size_t idx>
struct type_switch_impl<idx> {
  constexpr auto operator()(const auto&) const -> 
    std::expected<std::size_t, std::string> 
  {
    return std::unexpected("no matches found");
  }
};

// todo check if case and eval result match in terms of types
template <std::size_t idx, match_case_like match_case_head, match_case_like... match_case_rest>
struct type_switch_impl<idx, match_case_head, match_case_rest...> {
  constexpr auto operator()(const auto& v) const -> 
    std::expected<std::size_t, std::string> 
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
  using types_only = typelist::typelist<typename case_head::type_tag::type, typename case_rest::type_tag::type...>;
  using size_only = std::integer_sequence<std::size_t, case_head::type_tag::size, case_rest::type_tag::size...>;
  
  template <typename... fields>
  constexpr auto operator()(const auto& v) const -> 
    std::expected<std::size_t, std::string> 
  {
    return type_switch_impl<0, case_head, case_rest...>{}(v);
  } 
};

#endif // _TYPE_SWITCH_HPP_
