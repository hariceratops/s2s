#ifndef _FIELD_TYPE_HPP_
#define _FIELD_TYPE_HPP_


#include "sc_type_traits.hpp"
#include "field.hpp"
#include "field_size.hpp"
#include "type_deduction.hpp"
#include "compute_res.hpp"


struct always_true {
  constexpr auto operator()() -> bool {
    return true;
  }
};

using always_present = eval_bool_from_fields<always_true{}, with_fields<>>;

// Aliases
// todo enforce constraints wherever applicable
// todo maybe make expected, present_only_if, type_deduce as types
// todo rename expected -> value_constraint

// todo fixed_size_like
// template <
//   fixed_string id, 
//   integral T, 
//   fixed_size_like size, 
//   auto expected = no_constraint<T>{},
//   typename present_only_if = always_present,
//   typename type_deducer = type<no_type_deduction>
// >
// using basic_field = 
//   field<id, T, size, expected, present_only_if, type_deducer>;
//
// // todo T shall be fixed size
// template <
//   fixed_string id, 
//   typename T, 
//   std::size_t N, 
//   auto expected = no_constraint<std::array<T,N>>{},
//   typename present_only_if = always_present,
//   typename type_deducer = type<no_type_deduction>
// >
// using fixed_array_field = 
//   field<
//     id, 
//     std::array<T, N>, 
//     field_size<fixed<N * sizeof(T)>>, 
//     expected, 
//     present_only_if, 
//     type_deducer
//   >;
//
// template <
//   fixed_string id, 
//   std::size_t N,
//   auto expected = no_constraint<fixed_string<N + 1>>{},
//   typename present_only_if = always_present,
//   typename type_deducer = type<no_type_deduction>
// >
// using fixed_string_field = 
//   field<
//     id, 
//     fixed_string<N + 1>, 
//     field_size<fixed<N + 1>>,
//     expected, 
//     present_only_if, 
//     type_deducer
//   >;
//
// template <
//   fixed_string id, 
//   typename T, 
//   std::size_t N,
//   auto expected = no_constraint<T[N]>{},
//   typename present_only_if = always_present,
//   typename type_deducer = type<no_type_deduction>
// >
// using c_arr_field = 
//   field<
//     id, 
//     T[N], 
//     field_size<fixed<N * sizeof(T)>>,
//     expected, 
//     present_only_if, 
//     type_deducer
//   >;
//
// template <
//   fixed_string id, 
//   std::size_t N,
//   auto expected = no_constraint<char[N + 1]>{},
//   typename present_only_if = always_present,
//   typename type_deducer = type<no_type_deduction>
// >
// using c_str_field = 
//   field<
//     id, 
//     char[N + 1], 
//     field_size<fixed<N * sizeof(char) + 1>>,
//     expected, 
//     present_only_if, 
//     type_deducer
//   >;
//
// template <
//   fixed_string id, 
//   std::size_t N, 
//   std::array<unsigned char, N> expected,
//   typename present_only_if = always_present,
//   typename type_deducer = type<no_type_deduction>
// >
// using magic_byte_array = 
//   field<
//     id, 
//     std::array<unsigned char, N>, 
//     field_size<fixed<N>>, 
//     eq{expected},
//     present_only_if, 
//     type_deducer
//   >;
//
// template <
//   fixed_string id, 
//   fixed_string expected,
//   typename present_only_if = always_present,
//   typename type_deducer = type<no_type_deduction>
// >
// using magic_string = 
//   field<
//     id, 
//     fixed_string<expected.size()>, 
//     field_size<fixed<expected.size() + 1>>, 
//     eq(expected),
//     present_only_if, 
//     type_deducer
//   >;
//
// template <
//   fixed_string id, 
//   integral T, 
//   fixed_size_like size, 
//   T expected,
//   typename present_only_if = always_present,
//   typename type_deducer = type<no_type_deduction>
// >
// using magic_number = 
//   field<
//     id, 
//     T, 
//     size, 
//     eq{expected},
//     present_only_if, 
//     type_deducer
//   >;
//
// // digressions = What if user wants a custom allocator? use the plain version of the type instead of alias?
// // todo get vector length in bytes instead of size to read additional overload
// template <
//   fixed_string id, 
//   typename T, 
//   variable_size_like runtime_size,
//   auto expected = no_constraint<std::vector<T>>{},
//   typename present_only_if = always_present,
//   typename type_deducer = type<no_type_deduction>
// >
// using vec_field = 
//   field<
//     id, 
//     std::vector<T>, 
//     runtime_size,
//     expected,
//     present_only_if, 
//     type_deducer
// >;
//
// // todo check if this will work for all char types like wstring
// template <
//   fixed_string id, 
//   variable_size_like runtime_size,
//   auto expected = no_constraint<std::string>{},
//   typename present_only_if = always_present,
//   typename type_deducer = type<no_type_deduction>
// >
// using str_field = 
//   field<
//     id, 
//     std::string, 
//     runtime_size,
//     expected,
//     present_only_if, 
//     type_deducer
//   >;
//
// // todo: decide if maybe type cannot have type deduction and expected?
// template <
//   fixed_string id, 
//   typename T,
//   typename size,
//   typename present_only_if,
//   auto expected = no_constraint<std::optional<T>>{}
// >
// using maybe_field = 
//   field<
//     id, 
//     std::optional<T>, 
//     size,
//     expected,
//     present_only_if, 
//     type<no_type_deduction>
//   >;
//
// // todo: decide if a variant field be optional
// // todo constrains on type_deducer
// template <
//   fixed_string id, 
//   typename type_deducer,
//   auto expected = no_constraint<typename type_deducer::type_selection>{}
// >
// using union_field = 
//   field<
//     id, 
//     typename type_deducer::type_selection, 
//     typename type_deducer::size_selection,
//     expected,
//     always_present, 
//     type_deducer
//   >;
//
// template <
//   fixed_string id,
//   field_list_like T, 
//   typename present_only_if = always_present
// >
// using struct_field = 
//   field<
//     id, 
//     T, 
//     field_size<fixed<sizeof(T)>>, 
//     no_constraint<T>{}, 
//     present_only_if, 
//     type<no_type_deduction>
// >;
//
// /*
// *   type<
// *     eval_result<
// *       expression<callable>, 
// *       with_fields<...>, 
// *     >,
// *     type_switch<
// *       match_case<.., type_tag_1>,
// *       match_case<.., type_tag_2>
// *     >
// *   >
// *   */
// /*
// *   type<
// *     type_switch<
// *       eval_result<
// *         expression<callable>,
// *         with_fields<...>,
// *       >, type_tag>,
// *       ...
// *     >
// *   >
// *   */
//


// todo if fixed size and basic ensure constrint on size and data width
template <fixed_string id, integral T, fixed_size_like size_type, auto constraint_on_value = no_constraint<T>{}>
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

template <fixed_string id, typename T, variable_size_like size, auto constraint_on_value = no_constraint<std::vector<T>>{}>
using vec_field = field<id, std::vector<T>, size, constraint_on_value>;

template <fixed_string id, field_list_like T, variable_size_like size, auto constraint_on_value = no_constraint<std::vector<T>>{}>
using vector_of_records = field<id, std::vector<T>, size, constraint_on_value>;

template <fixed_string id, variable_size_like size, auto constraint_on_value = no_constraint<std::string>{}>
using str_field = field<id, std::string, size, constraint_on_value>;

template <fixed_string id, field_list_like T>
using struct_field = field<id, T, field_size<size_dont_care>, no_constraint<T>{}>;


namespace static_test {
  using u32 = unsigned int;
  static inline auto is_eq_1 = [](auto a){ return a == 1; };
  // static_assert(is_optional_field_v<maybe_field<"a", u32, field_size<fixed<4>>, parse_if<is_eq_1, with_fields<"a">>>>);
  // static_assert(!is_optional_field_v<basic_field<"a", u32, field_size<fixed<4>>>>);
}
#endif /* _FIELD_TYPE_HPP_ */
