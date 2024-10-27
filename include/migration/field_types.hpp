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

// todo comptime_size_like
template <
  fixed_string id, 
  integral T, 
  typename size, 
  auto expected = no_constraint<T>{},
  auto present_only_if = always_present{},
  auto type_deducer = type<no_type_deduction>{}
>
using integral_field = 
  field<id, T, size, expected, present_only_if, type_deducer>;

template <
  fixed_string id, 
  typename T, 
  std::size_t N, 
  auto expected = no_constraint<std::array<T,N>>{},
  auto present_only_if = always_present{},
  auto type_deducer = type<no_type_deduction>{}
>
using fixed_array_field = 
  field<
    id, 
    std::array<T, N>, 
    field_size<N * sizeof(T)>, 
    expected, 
    present_only_if, 
    type_deducer
  >;

template <
  fixed_string id, 
  std::size_t N,
  auto expected = no_constraint<fixed_string<N>>{},
  auto present_only_if = always_present{},
  auto type_deducer = type<no_type_deduction>{}
>
using fixed_string_field = 
  field<
    id, 
    fixed_string<N + 1>, 
    field_size<N + 1>,
    expected, 
    present_only_if, 
    type_deducer
  >;

template <
  fixed_string id, 
  typename T, 
  std::size_t N,
  auto expected = no_constraint<T[N]>{},
  auto present_only_if = always_present{},
  auto type_deducer = type<no_type_deduction>{}
>
using c_arr_field = 
  field<
    id, 
    T[N], 
    field_size<N * sizeof(T)>,
    expected, 
    present_only_if, 
    type_deducer
  >;

template <
  fixed_string id, 
  std::size_t N,
  auto expected = no_constraint<char[N + 1]>{},
  auto present_only_if = always_present{},
  auto type_deducer = type<no_type_deduction>{}
>
using c_str_field = 
  field<
    id, 
    char[N + 1], 
    field_size<N * sizeof(char) + 1>,
    expected, 
    present_only_if, 
    type_deducer
  >;

template <
  fixed_string id, 
  floating_point T,
  auto expected = no_constraint<T>{},
  auto present_only_if = always_present{},
  auto type_deducer = type<no_type_deduction>{}
>
using float_point_field = 
  field<
    id, 
    T, 
    field_size<sizeof(T)>,
    expected, 
    present_only_if, 
    type_deducer
  >;

template <
  fixed_string id, 
  std::size_t N, 
  std::array<unsigned char, N> expected,
  auto present_only_if = always_present{},
  auto type_deducer = type<no_type_deduction>{}
>
using magic_byte_array = 
  field<
    id, 
    std::array<unsigned char, N>, 
    field_size<N>, 
    eq{expected},
    present_only_if, 
    type_deducer
  >;

template <
  fixed_string id, 
  fixed_string expected,
  auto present_only_if = always_present{},
  auto type_deducer = type<no_type_deduction>{}
>
using magic_string = 
  field<
    id, 
    fixed_string<expected.size()>, 
    field_size<expected.size()>, 
    eq{expected},
    present_only_if, 
    type_deducer
  >;

template <
  fixed_string id, 
  integral T, 
  std::size_t size, 
  T expected,
  auto present_only_if = always_present{},
  auto type_deducer = type<no_type_deduction>{}
>
using magic_number = 
  field<
    id, 
    T, 
    field_size<size>, 
    eq{expected},
    present_only_if, 
    type_deducer
  >;

// digressions = What if user wants a custom allocator? use the plain version of the type instead of alias?
// todo get vector length in bytes instead of size to read additional overload
template <
  fixed_string id, 
  typename T, 
  typename runtime_size,
  auto expected = no_constraint<T>{},
  auto present_only_if = always_present{},
  auto type_deducer = type<no_type_deduction>{}
>
using vec_field = 
  field<
    id, 
    std::vector<T>, 
    runtime_size,
    expected,
    present_only_if, 
    type_deducer
>;

// todo check if this will work for all char types like wstring
template <
  fixed_string id, 
  typename runtime_size,
  auto expected = no_constraint<std::string>{},
  auto present_only_if = always_present{},
  auto type_deducer = type<no_type_deduction>{}
>
using str_field = 
  field<
    id, 
    std::string, 
    runtime_size,
    expected,
    present_only_if, 
    type_deducer
  >;

// todo: decide if maybe type cannot have type deduction and expected?
template <
  fixed_string id, 
  typename T,
  typename size,
  auto present_only_if,
  auto expected = no_constraint<std::string>{}
>
using maybe_field = 
  field<
    id, 
    std::optional<T>, 
    size,
    expected,
    present_only_if, 
    type<no_type_deduction>{}
  >;

// todo: decide if a variant field be optional
template <
  fixed_string id, 
  auto type_deducer,
  auto expected = no_constraint<std::string>{}
>
using union_field = 
  field<
    id, 
    std::string, 
    size,
    expected,
    always_present{}, 
    type_deducer
  >;

/*
*   type<
*     eval_result<
*       expression<callable>, 
*       with_fields<...>, 
*     >,
*     type_switch<
*       match_case<.., type_tag_1>,
*       match_case<.., type_tag_2>
*     >
*   >
*   */
/*
*   type<
*     type_switch<
*       eval_result<
*         expression<callable>,
*         with_fields<...>,
*       >, type_tag>,
*       ...
*     >
*   >
*   */

#endif /* _FIELD_TYPE_HPP_ */
