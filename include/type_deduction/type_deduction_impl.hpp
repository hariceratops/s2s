#ifndef _TYPE_DEDUCTION_IMPL_HPP_
#define _TYPE_DEDUCTION_IMPL_HPP_

#include <expected>
#include "../field/field_accessor.hpp"
#include "../error/cast_error.hpp"
#include "../field_list/field_list.hpp"
#include "../field_compute/computation_from_fields_impl.hpp"
#include "type_deduction.hpp"
#include "type_deduction_ladder_impl.hpp"
#include "type_deduction_switch_impl.hpp"


namespace s2s {
template <typename... Args>
struct deduce_type;


// todo constraints compute like
template <typename eval_expression, typename _switch>
struct deduce_type<type<eval_expression, _switch>> {
  template <auto metadata, typename... fields>
  constexpr auto operator()(const struct_field_list_impl<metadata, fields...>& sfl)
    -> std::expected<std::size_t, error_reason> const {
    return evaluate_switch<_switch>{}(compute_impl<eval_expression>{}(sfl)); 
  }
};

template <fixed_string id, typename _switch>
struct deduce_type<type<match_field<id>, _switch>> {
  template <auto metadata, typename... fields>
  constexpr auto operator()(const struct_field_list_impl<metadata, fields...>& sfl)
    -> std::expected<std::size_t, error_reason> const {
    return evaluate_switch<_switch>{}(sfl[field_accessor<id>{}]); 
  }
};

// todo constraints
template <typename ladder>
struct deduce_type<type<ladder>> {
  template <auto metadata, typename... fields>
  constexpr auto operator()(const struct_field_list_impl<metadata, fields...>& sfl)
    -> std::expected<std::size_t, error_reason> const {
    return evaluate_ladder<ladder>{}(sfl);
  }
};
} /* namespace s2s */


#endif // _TYPE_DEDUCTION_IMPL_HPP_
