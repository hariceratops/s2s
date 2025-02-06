#ifndef _FIELD_TRAITS_HPP_
#define _FIELD_TRAITS_HPP_


#include "field.hpp"
#include "field_size.hpp"


template <typename T>
struct is_fixed_sized_field;

// Specialization for field with fixed_size_like size
template <fixed_string id, typename T, fixed_size_like size, auto constraint_on_value>
struct is_fixed_sized_field<field<id, T, size, constraint_on_value>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_fixed_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_fixed_sized_field_v = is_fixed_sized_field<T>::res;

// Concept for fixed_sized_field_like
template <typename T>
concept fixed_sized_field_like = is_fixed_sized_field_v<T>;

template <typename T>
struct is_variable_sized_field;

// Specialization for field with variable_size_like size
template <fixed_string id, typename T, variable_size_like size, auto constraint_on_value>
struct is_variable_sized_field<field<id, T, size, constraint_on_value>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_variable_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_variable_sized_field_v = is_variable_sized_field<T>::res;

// Concept for variable_sized_field_like
template <typename T>
concept variable_sized_field_like = is_variable_sized_field_v<T>;


template <typename T>
struct is_optional_field;

// Specialization for maybe_field with a field
template <fixed_string id, 
          typename T, 
          typename size, 
          auto constraint_on_value, 
          typename present_only_if, 
          typename optional, 
          typename base_field>
struct is_optional_field<
    maybe_field<id, T, size, constraint_on_value, present_only_if, optional, base_field>
  > 
{
  static constexpr bool res = true;
};

template <typename T>
struct is_optional_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_optional_field_v = is_optional_field<T>::res;

// Concept for optional_field_like
template <typename T>
concept optional_field_like = is_optional_field_v<T>;


template <typename T>
struct is_union_field;

template <fixed_string id,
          typename type_deducer,
          typename type,
          typename size_type,
          auto constraint_on_value,
          typename variant,
          typename field_choices_t>
struct is_union_field<
    union_field<id, type_deducer, type, size_type, constraint_on_value, variant, field_choices_t>
  > 
{
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_union_field_v = is_union_field<T>::res;

template <typename T>
concept union_field_like = is_union_field_v<T>;

// todo struct_field_like

template <typename T>
concept field_like = fixed_sized_field_like<T> || 
                     variable_sized_field_like<T> || 
                     optional_field_like<T> || 
                     union_field_like<T>;
//
// namespace static_test {
//   static_assert(is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
// }
#endif /*_FIELD_TRAITS_HPP_*/
