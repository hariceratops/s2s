#ifndef _FIELD_TRAITS_HPP_
#define _FIELD_TRAITS_HPP_


#include "field.hpp"
#include "field_size.hpp"
#include "s2s_type_traits.hpp"


namespace s2s {
template <typename T>
struct is_fixed_sized_field;

// Specialization for field with fixed_size_like size
template <fixed_string id, field_containable T, fixed_size_like size, auto constraint_on_value>
struct is_fixed_sized_field<field<id, T, size, constraint_on_value>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_fixed_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_fixed_sized_field_v = is_fixed_sized_field<T>::res;

template <typename T>
concept fixed_sized_field_like = is_fixed_sized_field_v<T>;

template <typename T>
struct is_array_of_record_field;

template <fixed_string id, field_list_like T, std::size_t N, typename size, auto constraint_on_value>
struct is_array_of_record_field<field<id, std::array<T, N>, size, constraint_on_value>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_array_of_record_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_array_of_record_field_v = is_array_of_record_field<T>::res;

template <typename T>
concept array_of_record_field_like = is_array_of_record_field_v<T>;


template <typename T>
struct is_variable_sized_field;

// Specialization for field with variable_size_like size
template <fixed_string id, variable_sized_buffer_like T, variable_size_like size, auto constraint_on_value>
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
struct is_vector_of_record_field;

template <fixed_string id, field_list_like T, typename size, auto constraint_on_value>
struct is_vector_of_record_field<field<id, std::vector<T>, size, constraint_on_value>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_vector_of_record_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_vector_of_record_field_v = is_vector_of_record_field<T>::res;

template <typename T>
concept vector_of_record_field_like = is_vector_of_record_field_v<T>;

template <typename T>
struct is_struct_field;

// Specialization for field with variable_size_like size
template <fixed_string id, field_list_like T, typename size, auto constraint_on_value>
struct is_struct_field<field<id, T, size, constraint_on_value>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_struct_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_struct_field_v = is_struct_field<T>::res;

// Concept for variable_sized_field_like
template <typename T>
concept struct_field_like = is_struct_field_v<T>;


template <typename T>
struct is_optional_field;

// Specialization for maybe_field with a field
template <fixed_string id, 
          typename T, 
          typename size, 
          auto constraint, 
          typename present_only_if, 
          typename optional>
struct is_optional_field<
    maybe_field<field<id, T, size, constraint>, present_only_if, optional>
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

template <fixed_string id, typename type_deducer>
struct is_union_field<
    union_field<id, type_deducer>
  > 
{
  static constexpr bool res = true;
};

template <typename T>
struct is_union_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_union_field_v = is_union_field<T>::res;

template <typename T>
concept union_field_like = is_union_field_v<T>;

template <typename T>
concept field_like = fixed_sized_field_like<T> || 
                     variable_sized_field_like<T> ||
                     array_of_record_field_like<T> ||
                     vector_of_record_field_like<T> ||
                     struct_field_like<T> || 
                     optional_field_like<T> || 
                     union_field_like<T>;
} /* namespace s2s */

#endif /*_FIELD_TRAITS_HPP_*/
