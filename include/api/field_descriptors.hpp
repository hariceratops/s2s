#ifndef _FIELD_DESCRIPTORS_HPP_
#define _FIELD_DESCRIPTORS_HPP_


#include "../lib/s2s_traits/type_traits.hpp"
#include "../field/field.hpp"
#include "../field/field_options.hpp"
#include "../field_list/field_list_base.hpp"
#include "../field_size/field_size.hpp"
#include "../field_size/field_size_deduce.hpp"
#include "../field_validation/field_value_constraints.hpp"
#include "../type_deduction/type/type_deduction_traits.hpp"
#include "../type_deduction/type/type_deduction_metafunctions.hpp"


namespace s2s {
struct always_true {
  // const: compute_impl invokes the callable as a const NTTP, so without this
  // always_present fails to compile in either direction.
  constexpr auto operator()() const -> bool {
    return true;
  }
};

using always_present = eval_bool_from_fields<always_true{}>;

template <fixed_string id, integral T, field_option_like<T> auto... opts>
  requires field_fits_to_underlying_type<size_of_pack<T, opts...>, T>
using basic_field = field<id, T, size_of_pack<T, opts...>, constraint_of_pack<T, opts...>>;

template <fixed_string id, field_containable T, std::size_t N,
          constraint_option_like<std::array<T, N>> auto... opts>
using fixed_array_field =
  field<id, std::array<T, N>, byte_count{N * sizeof(T)}, constraint_of_pack<std::array<T, N>, opts...>>;

template <fixed_string id, field_list_like T, std::size_t N,
          constraint_option_like<std::array<T, N>> auto... opts>
using array_of_records =
  field<id, std::array<T, N>, size_dont_care, constraint_of_pack<std::array<T, N>, opts...>>;

template <fixed_string id, std::size_t N, constraint_option_like<fixed_string<N>> auto... opts>
using fixed_string_field =
  field<id, fixed_string<N>, byte_count{N + 1}, constraint_of_pack<fixed_string<N>, opts...>>;

template <fixed_string id, field_containable T, std::size_t N,
          constraint_option_like<T[N]> auto... opts>
using c_arr_field = field<id, T[N], byte_count{N * sizeof(T)}, constraint_of_pack<T[N], opts...>>;

template <fixed_string id, std::size_t N, constraint_option_like<char[N + 1]> auto... opts>
using c_str_field =
  field<id, char[N + 1], byte_count{N * sizeof(char) + 1}, constraint_of_pack<char[N + 1], opts...>>;

template <fixed_string id, std::size_t N, auto expected>
using magic_byte_array = field<id, std::array<unsigned char, N>, byte_count{N}, eq{expected}>;

template <fixed_string id, fixed_string expected>
using magic_string = field<id, fixed_string<expected.size()>, byte_count{expected.size() + 1}, eq{expected}>;

template <fixed_string id, integral T, auto size, auto expected>
  requires fixed_size_like<size_type_of<size>>
using magic_number = field<id, T, size, eq{expected}>;

// todo how user can provide user defined vector impl or allocator
template <fixed_string id, typename T, boundable_field_option_like<std::vector<T>> auto... opts>
  requires variable_size_like<size_type_of<size_of_pack<std::vector<T>, opts...>>>
using vec_field =
  field<id, std::vector<T>, size_of_pack<std::vector<T>, opts...>,
        constraint_of_pack<std::vector<T>, opts...>,
        bound_of_pack<std::vector<T>, opts...>>;

template <fixed_string id, field_list_like T, boundable_field_option_like<std::vector<T>> auto... opts>
  requires variable_size_like<size_type_of<size_of_pack<std::vector<T>, opts...>>>
using vector_of_records =
  field<id, std::vector<T>, size_of_pack<std::vector<T>, opts...>,
        constraint_of_pack<std::vector<T>, opts...>,
        bound_of_pack<std::vector<T>, opts...>>;

// todo check if this will work for all char types like wstring
template <fixed_string id, boundable_field_option_like<std::string> auto... opts>
  requires variable_size_like<size_type_of<size_of_pack<std::string, opts...>>>
using str_field =
  field<id, std::string, size_of_pack<std::string, opts...>,
        constraint_of_pack<std::string, opts...>,
        bound_of_pack<std::string, opts...>>;

template <fixed_string id, field_list_like T, constraint_option_like<T> auto... opts>
using struct_field = field<id, T, size_dont_care, constraint_of_pack<T, opts...>>;

template <no_variance_field_like base_field, typename present_only_if>
  requires is_eval_bool_from_fields_v<present_only_if>
using maybe = maybe_field<base_field, present_only_if>;


// Constraint-only, and per-element: a union's own size is size_dont_care and it
// drives no allocation of its own, so a size or a bound entry here has nothing
// to act on and fails the placeholder constraint.
template <fixed_string id, type_deduction_like type_deducer,
          constraint_option_like<typename type_deducer::variant> auto... opts>
  requires (has_unique_field_choices(extract_field_choices<type_deducer>::value)) &&
           (has_unique_match_values(extract_match_values<type_deducer>::value))
using variance =
  union_field<id, type_deducer,
              constraint_of_pack<typename type_deducer::variant, opts...>>;

} /* namespace s2s */

#endif /* _FIELD_DESCRIPTORS_HPP_ */
