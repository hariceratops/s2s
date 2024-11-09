#ifndef _FIELD_TRAITS_HPP_
#define _FIELD_TRAITS_HPP_


#include "field.hpp"
#include "field_size.hpp"
#include "sc_type_traits.hpp"


template <typename T>
struct is_field;

template <typename T>
struct is_field: std::false_type{};

template <fixed_string id, typename T, typename field_size, auto constraint, auto presence_check, auto type_deducer>
struct is_field<field<id, T, field_size, constraint, presence_check, type_deducer>>: std::true_type{};

template <typename T>
constexpr bool is_field_v = is_field<T>::value;


template <typename T>
struct is_comptime_sized_field;

template <fixed_string id,
          field_containable T, 
          comptime_size_like size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_comptime_sized_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_sized_field_v = is_comptime_sized_field<T>::res;

template <typename T>
struct is_runtime_sized_field;

// todo: todo var buffer like field constraint
template <fixed_string id,
          typename T, 
          runtime_size_like size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_runtime_sized_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_runtime_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_runtime_sized_field_v = is_runtime_sized_field<T>::res;

template <typename T>
struct is_optional_field;

template <fixed_string id,
          optional_like T, 
          typename size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_optional_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_optional_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_optional_field_v = is_optional_field<T>::res;

template <typename T>
struct is_union_field;

template <typename T>
struct is_union_field {
  static constexpr bool res = false;
};

template <fixed_string id,
          variant_like T, 
          typename size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_union_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_union_field_v = is_union_field<T>::res;

template <typename T>
concept field_like = is_comptime_sized_field_v<T> ||
                     is_runtime_sized_field_v<T>  ||
                     is_optional_field_v<T>       ||
                     is_union_field_v<T>;

// namespace static_test {
//   static_assert(is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
// }

#endif /*_FIELD_TRAITS_HPP_*/
