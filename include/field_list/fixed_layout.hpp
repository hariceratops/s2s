#ifndef _FIXED_LAYOUT_HPP_
#define _FIXED_LAYOUT_HPP_

#include <array>
#include <cstddef>
#include <string_view>
#include <type_traits>
#include "field_list.hpp"
#include "field_list_base.hpp"
#include "../field/field_metafunctions.hpp"
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

template <typename T>
inline constexpr bool is_array_of_trivials_v = false;

template <trivial T, std::size_t N>
inline constexpr bool is_array_of_trivials_v<std::array<T, N>> = true;

template <typename T>
inline constexpr bool is_c_array_of_trivials_v = false;

template <trivial T, std::size_t N>
inline constexpr bool is_c_array_of_trivials_v<T[N]> = true;

template <typename T>
concept array_of_trivials = is_array_of_trivials_v<T>;

template <typename T>
concept c_array_of_trivials = is_c_array_of_trivials_v<T>;

// What a view can return for a fixed-width leaf: a scalar, a whole array, a C
// array (as its std::array equivalent) or a fixed string.
template <typename T>
concept viewable_leaf_value =
  trivial<T> || array_of_trivials<T> || c_array_of_trivials<T> || fixed_string_like<T>;

template <typename F>
inline constexpr bool leaf_has_compile_time_width_v =
  fixed_sized_field_like<F> &&
  viewable_leaf_value<typename F::field_type>;

template <typename F>
inline constexpr bool field_is_constrained_v =
  !is_no_constraint_v<std::remove_cvref_t<decltype(F::constraint_checker)>>;

template <typename S>
inline constexpr bool any_field_constrained_v = false;

// A record with no constraint of its own still needs a walk when something
// inside it is constrained.
template <typename F>
inline constexpr bool field_needs_check_v = field_is_constrained_v<F>;

template <struct_field_like F>
inline constexpr bool field_needs_check_v<F> =
  field_is_constrained_v<F> || any_field_constrained_v<extract_type_from_field_v<F>>;

// Lets creation skip the constraint walk entirely for a schema with nothing to check.
template <auto metadata, typename... fields>
inline constexpr bool any_field_constrained_v<struct_field_list_impl<metadata, fields...>> =
  (field_needs_check_v<fields> || ...);

template <typename S>
inline constexpr bool every_field_has_compile_time_width_v = false;

template <typename F>
inline constexpr bool field_has_compile_time_width_v = leaf_has_compile_time_width_v<F>;

template <struct_field_like F>
inline constexpr bool field_has_compile_time_width_v<F> =
  every_field_has_compile_time_width_v<extract_type_from_field_v<F>>;

template <auto metadata, typename... fields>
inline constexpr bool every_field_has_compile_time_width_v<struct_field_list_impl<metadata, fields...>> =
  (field_has_compile_time_width_v<fields> && ...);

template <typename S>
concept every_field_has_compile_time_width = every_field_has_compile_time_width_v<S>;

template <typename S>
concept fixed_layout_schema =
  field_list_like<S> &&
  every_field_has_compile_time_width<S>;


template <typename S>
struct record_layout;

template <typename F>
constexpr auto field_wire_width() -> std::size_t {
  if constexpr(struct_field_like<F>)
    return record_layout<extract_type_from_field_v<F>>::width;
  else
    return deduce_field_size<F::field_size>{}();
}

template <auto metadata, typename... fields>
struct record_layout<struct_field_list_impl<metadata, fields...>> {
  static constexpr std::size_t field_count = sizeof...(fields);

  static constexpr std::array<std::size_t, field_count> widths{
    field_wire_width<fields>()...
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
