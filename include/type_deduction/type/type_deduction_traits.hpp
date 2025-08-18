#ifndef _TYPE_DEDUCTION_TRAITS_HPP_
#define _TYPE_DEDUCTION_TRAITS_HPP_

#include "../../field_compute/computation_from_fields.hpp"
#include "../switch/switch_traits.hpp"
#include "../if_else_ladder/ladder_traits.hpp"
#include "type.hpp"


namespace s2s {

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

template <typename eval_expression, typename _switch>
struct is_type_deduction<type<eval_expression, _switch>> {
  static constexpr bool res = 
    is_compute_like_v<eval_expression> && 
    type_switch_like<_switch>;
};

template <fixed_string id, typename _switch>
struct is_type_deduction<type<match_field<id>, _switch>> {
  static constexpr bool res = type_switch_like<_switch>;
};

template <typename ladder>
struct is_type_deduction<type<ladder>> {
  static constexpr bool res = type_if_else_like<ladder>;
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


#endif // _TYPE_DEDUCTION_TRAITS_HPP_
