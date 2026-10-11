#ifndef _FIXED_LAYOUT_HPP_
#define _FIXED_LAYOUT_HPP_

#include <array>
#include <cstddef>
#include <string_view>
#include <type_traits>
#include "field_list.hpp"
#include "field_list_base.hpp"
#include "../field/field_traits.hpp"
#include "../field_size/comptime_field_size_deduce.hpp"
#include "../field_validation/field_value_constraints.hpp"
#include "../lib/s2s_traits/type_traits.hpp"


namespace s2s {
// Every _v atom has a primary of false so each sub-concept is pure for any
// argument. Each concept body is exactly one atom whose name is the rule: gcc
// prints the failing atom, not the concept, at its default diagnostic depth.
// The concept is deliberately free of static_assert (unlike fixed_order_schema)
// so that it can be tested as false.

// Narrowed per slice (design 2.5): a constrained or non-trivial leaf is not
// admitted until the code that honours it exists.
template <typename F>
inline constexpr bool leaf_has_compile_time_width_v =
  fixed_sized_field_like<F> &&
  trivial<typename F::field_type> &&
  is_no_constraint_v<std::remove_cvref_t<decltype(F::constraint_checker)>>;

template <typename S>
inline constexpr bool every_field_has_compile_time_width_v = false;

template <auto metadata, typename... fields>
inline constexpr bool every_field_has_compile_time_width_v<struct_field_list_impl<metadata, fields...>> =
  (leaf_has_compile_time_width_v<fields> && ...);

template <typename S>
concept every_field_has_compile_time_width = every_field_has_compile_time_width_v<S>;

template <typename S>
concept fixed_layout_schema =
  field_list_like<S> &&
  every_field_has_compile_time_width<S>;


template <typename S>
struct record_layout;

template <auto metadata, typename... fields>
struct record_layout<struct_field_list_impl<metadata, fields...>> {
  static constexpr std::size_t field_count = sizeof...(fields);

  static constexpr std::array<std::size_t, field_count> widths{
    deduce_field_size<fields::field_size>{}()...
  };

  static constexpr std::array<std::size_t, field_count> offsets = [] {
    std::array<std::size_t, field_count> res{};
    std::size_t running = 0;
    for(std::size_t idx = 0; idx < field_count; ++idx) {
      res[idx] = running;
      running += widths[idx];
    }
    return res;
  }();

  static constexpr std::size_t width = [] {
    std::size_t sum = 0;
    for(auto w: widths)
      sum += w;
    return sum;
  }();

  // Same spelling the parser reports in failed_at.
  static constexpr std::array<std::string_view, field_count> names{
    std::string_view{fields::field_id.data()}...
  };
};

template <fixed_layout_schema S>
inline constexpr std::size_t wire_width = record_layout<S>::width;
} /* namespace s2s */

#endif // _FIXED_LAYOUT_HPP_
