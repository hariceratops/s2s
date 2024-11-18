#ifndef _TYPE_DEDUCTION_HPP_
#define _TYPE_DEDUCTION_HPP_

#include <variant>
#include <expected>
#include "field_accessor.hpp"
#include "typelist.hpp"
#include "type_ladder.hpp"
#include "type_switch.hpp"


template <typename... clauses>
struct clauses_to_typelist {
  using tlist = typelist::typelist<typename clauses::type_tag...>;
  // todo aargh, variable length types might have to computed at cast function
};

struct no_type_deduction {};

template <typename T>
struct is_no_type_deduction;

template <typename T>
struct is_no_type_deduction {
  static constexpr bool res = false;
};

template <>
struct is_no_type_deduction<no_type_deduction> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_no_type_deduction_v = is_no_type_deduction<T>::res;

template <typename T>
concept no_type_deduction_like = is_no_type_deduction_v<T>;


template <typename... Args>
struct type;

template <no_type_deduction_like T>
struct type<T> {};


template <fixed_string id>
using match_field = field_accessor<id>;

// todo constraints compute like
template <typename eval_expression, typename tswitch>
struct type<eval_expression, tswitch> {
  using expression = eval_expression;
  using type_switch = tswitch;
  using type_selection = tswitch::types_only;
  using size_selection = tswitch::size_only;

  template <typename... fields>
  auto operator()(const struct_field_list<fields...>& sfl)
    -> std::expected<std::size_t, std::string> const {
    return type_switch{}(eval_expression{}(sfl)); 
  }
};

template <fixed_string id, typename tswitch>
struct type<match_field<id>, tswitch> {
  using type_switch = tswitch;
  using type_selection = tswitch::types_only;
  using size_selection = tswitch::size_only;

  template <typename... fields>
  auto operator()(const struct_field_list<fields...>& sfl)
    -> std::expected<std::size_t, std::string> const {
    return type_switch{}(sfl[field_accessor<id>{}]); 
  }
};

// todo constraints
template <typename tladder>
struct type<tladder> {
  using type_ladder = tladder;
  using type_selection = tladder::types_only;
  using size_selection = tladder::size_only;

  template <typename... fields>
  auto operator()(const struct_field_list<fields...>& sfl)
    -> std::expected<std::size_t, std::string> const {
    return type_ladder{}(sfl);
  }
};

// todo metafunction and concepts for constraining type

#endif // _TYPE_DEDUCTION_HPP_
