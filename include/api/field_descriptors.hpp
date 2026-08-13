#ifndef _FIELD_DESCRIPTORS_HPP_
#define _FIELD_DESCRIPTORS_HPP_


#include "../lib/s2s_traits/type_traits.hpp"
#include "../field/field.hpp"
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

using always_present = eval_bool_from_fields<always_true{}, with_fields<>>;

template <auto size, typename field_type>
concept field_fits_to_underlying_type = deduce_field_size<size>{}() <= sizeof(field_type);

template <fixed_string id, integral T, auto size, auto constraint_on_value = no_constraint<T>{}>
  requires fixed_size_like<size_type_of<size>> && field_fits_to_underlying_type<size, T>
using basic_field = field<id, T, size, constraint_on_value>;

template <fixed_string id, field_containable T, std::size_t N, auto constraint_on_value = no_constraint<std::array<T, N>>{}>
using fixed_array_field = field<id, std::array<T, N>, field_size<fixed<N * sizeof(T)>>, constraint_on_value>;

template <fixed_string id, field_list_like T, std::size_t N, auto constraint_on_value = no_constraint<std::array<T, N>>{}>
using array_of_records = field<id, std::array<T, N>, field_size<size_dont_care>, constraint_on_value>;

template <fixed_string id, std::size_t N, auto constraint_on_value = no_constraint<fixed_string<N>>{}>
using fixed_string_field = field<id, fixed_string<N>, field_size<fixed<N + 1>>, constraint_on_value>;

template <fixed_string id, field_containable T, std::size_t N, auto constraint_on_value = no_constraint<T[N]>{}>
using c_arr_field = field<id, T[N], field_size<fixed<N * sizeof(T)>>, constraint_on_value>;

template <fixed_string id, std::size_t N, auto constraint_on_value = no_constraint<char[N + 1]>{}>
using c_str_field = field<id, char[N + 1], field_size<fixed<N * sizeof(char) + 1>>, constraint_on_value>;

template <fixed_string id, std::size_t N, auto expected>
using magic_byte_array = field<id, std::array<unsigned char, N>, field_size<fixed<N>>, eq{expected}>;

template <fixed_string id, fixed_string expected>
using magic_string = field<id, fixed_string<expected.size()>, field_size<fixed<expected.size() + 1>>, eq{expected}>;

template <fixed_string id, integral T, auto size, auto expected>
  requires fixed_size_like<size_type_of<size>>
using magic_number = field<id, T, size, eq{expected}>;

// todo how user can provide user defined vector impl or allocator
template <fixed_string id, typename T, auto size, auto constraint_on_value = no_constraint<std::vector<T>>{}>
  requires variable_size_like<size_type_of<size>>
using vec_field = field<id, std::vector<T>, size, constraint_on_value>;

template <fixed_string id, field_list_like T, auto size, auto constraint_on_value = no_constraint<std::vector<T>>{}>
  requires variable_size_like<size_type_of<size>>
using vector_of_records = field<id, std::vector<T>, size, constraint_on_value>;

// todo check if this will work for all char types like wstring
template <fixed_string id, auto size, auto constraint_on_value = no_constraint<std::string>{}>
  requires variable_size_like<size_type_of<size>>
using str_field = field<id, std::string, size, constraint_on_value>;

template <fixed_string id, field_list_like T>
using struct_field = field<id, T, field_size<size_dont_care>, no_constraint<T>{}>;

template <no_variance_field_like base_field, typename present_only_if>
  requires is_eval_bool_from_fields_v<present_only_if>
using maybe = maybe_field<base_field, present_only_if>;


template <fixed_string id, type_deduction_like type_deducer>
  requires (has_unique_field_choices(extract_field_choices<type_deducer>::value)) &&
           (has_unique_match_values(extract_match_values<type_deducer>::value))
using variance = union_field<id, type_deducer>;

} /* namespace s2s */

#endif /* _FIELD_DESCRIPTORS_HPP_ */
