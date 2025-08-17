#ifndef _TYPE_DEDUCTION_IMPL_HPP_
#define _TYPE_DEDUCTION_IMPL_HPP_

#include <expected>
#include "../field/field_accessor.hpp"
#include "../error/cast_error.hpp"
#include "../field_list/field_list.hpp"
#include "type_deduction.hpp"


namespace s2s {
template <typename... Args>
struct deduce_type;


template <fixed_string id>
using match_field = field_accessor<id>;

// todo constraints compute like
template <typename eval_expression, typename _switch>
struct deduce_type<type<eval_expression, _switch>> {
  using expression = eval_expression;
  using type_switch = _switch;
  using variant = _switch::variant;
  using sizes = _switch::sizes;

  template <typename... fields>
  constexpr auto operator()(const struct_field_list_impl<fields...>& sfl)
    -> std::expected<std::size_t, error_reason> const {
    return type_switch{}(eval_expression{}(sfl)); 
  }
};

template <fixed_string id, typename _switch>
struct deduce_type<type<match_field<id>, _switch>> {
  using type_switch = _switch;
  using variant = _switch::variant;
  using sizes = _switch::sizes;

  template <typename... fields>
  constexpr auto operator()(const struct_field_list_impl<fields...>& sfl)
    -> std::expected<std::size_t, error_reason> const {
    return type_switch{}(sfl[field_accessor<id>{}]); 
  }
};

// todo constraints
template <typename ladder>
struct deduce_type<type<ladder>> {
  using type_ladder = ladder;
  using variant = ladder::variant;
  using sizes = ladder::sizes;

  template <typename... fields>
  constexpr auto operator()(const struct_field_list_impl<fields...>& sfl)
    -> std::expected<std::size_t, error_reason> const {
    return type_ladder{}(sfl);
  }
};
} /* namespace s2s */


#endif // _TYPE_DEDUCTION_IMPL_HPP_
