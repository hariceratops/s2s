#ifndef _FIELD_DESCRIPTORS_HPP_
#define _FIELD_DESCRIPTORS_HPP_


#include "../field_list/field_list_metadata.hpp"
#include "../lib/s2s_traits/type_traits.hpp"
#include "../field/field.hpp"
#include "../field_list/field_list_base.hpp"
#include "../field_list/field_list.hpp"
#include "../field_size/field_size.hpp"
#include "../field_size/field_size_deduce.hpp"
#include "../field_validation/field_value_constraints.hpp"
#include "../field_compute/computation_from_fields.hpp"
#include "../type_deduction/type_deduction.hpp"
#include "../lib/algorithms/algorithms.hpp"
#include "../lib/containers/static_array.hpp"
#include "../lib/containers/static_set.hpp"
#include "../type_deduction/type_deduction_metafunctions.hpp"


namespace s2s {
struct always_true {
  constexpr auto operator()() -> bool {
    return true;
  }
};

using always_present = eval_bool_from_fields<always_true{}, with_fields<>>;

template <typename size, typename field_type>
concept field_fits_to_underlying_type = deduce_field_size<size>{}() <= sizeof(field_type);

template <fixed_string id, integral T, fixed_size_like size_type, auto constraint_on_value = no_constraint<T>{}>
  requires field_fits_to_underlying_type<size_type, T>
using basic_field = field<id, T, size_type, constraint_on_value>;

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

template <fixed_string id, integral T, fixed_size_like size, auto expected>
using magic_number = field<id, T, size, eq{expected}>;

// todo how user can provide user defined vector impl or allocator
template <fixed_string id, typename T, variable_size_like size, auto constraint_on_value = no_constraint<std::vector<T>>{}>
using vec_field = field<id, std::vector<T>, size, constraint_on_value>;

template <fixed_string id, field_list_like T, variable_size_like size, auto constraint_on_value = no_constraint<std::vector<T>>{}>
using vector_of_records = field<id, std::vector<T>, size, constraint_on_value>;

// todo check if this will work for all char types like wstring
template <fixed_string id, variable_size_like size, auto constraint_on_value = no_constraint<std::string>{}>
using str_field = field<id, std::string, size, constraint_on_value>;

template <fixed_string id, field_list_like T>
using struct_field = field<id, T, field_size<size_dont_care>, no_constraint<T>{}>;

template <no_variance_field_like base_field, typename present_only_if>
  requires is_eval_bool_from_fields_v<present_only_if>
using maybe = maybe_field<base_field, present_only_if>;


template <fixed_string id, type_deduction_like type_deducer>
  requires (are_type_ids_unique(extract_field_choices<type_deducer>::value))
using variance = union_field<id, type_deducer>;

template <typename... fields>
concept all_field_like = (field_like<fields> && ...);

template <std::size_t N>
constexpr bool are_field_ids_unique(const std::array<std::string_view, N>& field_id_list) {
  static_set<std::string_view, N> field_id_set(field_id_list);
  return equal_ranges(field_id_list, field_id_set);
}

template <std::size_t N>
constexpr auto as_sv(const fixed_string<N>& str) {
  return std::string_view{str.data()};
}

template <typename... fields>
concept has_unique_field_ids = are_field_ids_unique(std::array{as_sv(fields::field_id)...});

template <typename metadata>
struct dependency_check {
  static constexpr bool size_ok = size_dependencies_resolved<metadata>();
  static constexpr bool parse_ok = parse_dependencies_resolved<metadata>();
  static constexpr bool type_ok = type_deduction_dependencies_resolved<metadata>();

  static_assert(size_ok, "Size dependencies not resolved");
  static_assert(parse_ok, "Parse dependencies not resolved");
  static_assert(type_ok, "Type deduction dependencies not resolved");

  static constexpr bool all_ok = size_ok && parse_ok && type_ok;
};

template <typename metadata>
concept all_dependencies_resolved = dependency_check<metadata>::all_ok;

template <typename... fields>
  requires (all_dependencies_resolved<field_list_metadata<fields...>>)
struct create_struct_field_list {
  using metadata = field_list_metadata<fields...>;
  using value = struct_field_list_impl<metadata, fields...>;
};

template <typename... fields>
  requires all_field_like<fields...> &&
           has_unique_field_ids<fields...>
using struct_field_list = create_struct_field_list<fields...>::value;

} /* namespace s2s */

#endif /* _FIELD_DESCRIPTORS_HPP_ */
