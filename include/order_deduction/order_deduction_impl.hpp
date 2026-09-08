#ifndef _ORDER_DEDUCTION_IMPL_HPP_
#define _ORDER_DEDUCTION_IMPL_HPP_


#include <array>
#include <bit>
#include <expected>

#include "../error/cast_error.hpp"
#include "../field/field_accessor.hpp"
#include "../field_list/field_list.hpp"
#include "../type_deduction/switch/switch_impl.hpp"
#include "order_deduction.hpp"


namespace s2s {
// evaluate_switch_helper folds a positional case list down to an index, and an
// index is what a case list resolves to on either axis — only the mapping out
// of it differs. Reused as it stands rather than reimplemented.
//
// A marker matching nothing is validation_failure, not the type axis's
// type_deduction_failure. On the type axis a failed deduction means the reader
// cannot proceed at all; here it means the file is not this format, which is a
// magic mismatch, and that is validation_failure everywhere else in s2s.
template <typename _switch>
struct evaluate_order_switch;

template <typename... cases>
struct evaluate_order_switch<order_switch<cases...>> {
  constexpr auto operator()(const auto& v) const -> std::expected<std::endian, error_reason> {
    auto res =
      evaluate_switch_helper<cases...>{}(
        v, std::make_index_sequence<sizeof...(cases)>{}
      );
    if(!res)
      return std::unexpected(error_reason::validation_failure);
    constexpr auto orders = std::array{cases::byte_order...};
    return orders[*res];
  }
};


template <typename guide>
struct deduce_order;

template <fixed_string id, typename _switch>
struct deduce_order<order_from<match_field<id>, _switch>> {
  template <auto metadata, typename... fields>
  constexpr auto operator()(const struct_field_list_impl<metadata, fields...>& sfl) const
    -> std::expected<std::endian, error_reason> {
    return evaluate_order_switch<_switch>{}(sfl[field_accessor<id>{}]);
  }
};
} /* namespace s2s */


#endif // _ORDER_DEDUCTION_IMPL_HPP_
