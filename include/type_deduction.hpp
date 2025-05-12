#ifndef _TYPE_DEDUCTION_HPP_
#define _TYPE_DEDUCTION_HPP_

#include <expected>
#include "field_accessor.hpp"
#include "typelist.hpp"
#include "cast_error.hpp"
#include "type_deduction_ladder.hpp"
#include "type_deduction_switch.hpp"


namespace s2s {
template <typename... Args>
struct type;


template <fixed_string id>
using match_field = field_accessor<id>;

// todo constraints compute like
template <typename eval_expression, typename tswitch>
struct type<eval_expression, tswitch> {
  using expression = eval_expression;
  using type_switch = tswitch;
  using variant = tswitch::variant;
  using sizes = tswitch::sizes;

  template <typename... fields>
  constexpr auto operator()(const struct_field_list<fields...>& sfl)
    -> std::expected<std::size_t, error_reason> const {
    return type_switch{}(eval_expression{}(sfl)); 
  }
};

template <fixed_string id, typename tswitch>
struct type<match_field<id>, tswitch> {
  using type_switch = tswitch;
  using variant = tswitch::variant;
  using sizes = tswitch::sizes;

  template <typename... fields>
  constexpr auto operator()(const struct_field_list<fields...>& sfl)
    -> std::expected<std::size_t, error_reason> const {
    return type_switch{}(sfl[field_accessor<id>{}]); 
  }
};

// todo constraints
template <typename tladder>
struct type<tladder> {
  using type_ladder = tladder;
  using variant = tladder::variant;
  using sizes = tladder::sizes;

  template <typename... fields>
  constexpr auto operator()(const struct_field_list<fields...>& sfl)
    -> std::expected<std::size_t, error_reason> const {
    return type_ladder{}(sfl);
  }
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


template <typename T>
struct is_type_deduction;

template <>
struct is_type_deduction<no_type_deduction> {
  static constexpr bool res = true;
};

template <typename eval_expression, typename tswitch>
struct is_type_deduction<type<eval_expression, tswitch>> {
  static constexpr bool res = is_compute_like_v<eval_expression> && 
                              type_switch_like<tswitch>;
};

template <fixed_string id, typename tswitch>
struct is_type_deduction<type<match_field<id>, tswitch>> {
  static constexpr bool res = type_switch_like<tswitch>;
};

template <typename tladder>
struct is_type_deduction<type<tladder>> {
  static constexpr bool res = type_ladder_like<tladder>;
};

template <typename T>
struct is_type_deduction {
  static constexpr bool res = false;
};

template <typename T>
static constexpr bool is_type_deduction_v = is_type_deduction<T>::res;

template <typename T>
concept type_deduction_like = is_type_deduction_v<T>;
} /* namespace s2s */


#endif // _TYPE_DEDUCTION_HPP_
