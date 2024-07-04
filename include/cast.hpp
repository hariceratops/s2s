#ifndef _CAST_HPP_
#define _CAST_HPP_

#include "field_meta.hpp"

template <typename T>
struct is_field;

template <typename T>
struct is_field: std::false_type{};

template <fixed_string id, typename T, typename field_size, auto constraint>
struct is_field<field<id, T, field_size, constraint>>: std::true_type{};

template <typename T>
constexpr bool is_field_v = is_field<T>::value;

template <typename T>
struct is_field_with_runtime_size;

template <typename T>
struct is_field_with_runtime_size: std::false_type{};

template <fixed_string id, typename T, typename field_size, auto constraint>
struct is_field_with_runtime_size<runtime_field<id, T, field_size, constraint>>: std::true_type{};

template <typename T>
constexpr bool is_field_with_runtime_size_v = is_field_with_runtime_size<T>::value;

static_assert(is_field_with_runtime_size_v<runtime_field<"hello", int, runtime_size<from_field<"a">>>>);
static_assert(!is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
static_assert(is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
static_assert(!is_field_v<runtime_field<"hello", int, runtime_size<from_field<"a">>>>);

template <typename... fields>
struct struct_field_list;

// Metafunction to check if a type is a struct_field_list
template <typename T>
struct is_struct_field_list : std::false_type {};

template <typename... Entries>
struct is_struct_field_list<struct_field_list<Entries...>> : std::true_type {};

template <typename T>
constexpr bool is_struct_field_list_v = is_struct_field_list<T>::value;


template <typename... fields>
constexpr void struct_cast(struct_field_list<fields...>& field_list, const unsigned char* buffer) {
  std::size_t prefix_sum[sizeof...(fields) + 1] = {0};
  std::size_t index = 0;

  ([&](auto& field) {
    using field_type = std::decay_t<decltype(field)>;
    prefix_sum[index + 1] = prefix_sum[index] + field_type::field_size;
    if constexpr (is_struct_field_list_v<extract_type_from_field_v<field_type>>) {
      struct_cast(field.value, buffer + prefix_sum[index]);
    } else if constexpr (is_field_v<field_type>) {
      field.read(reinterpret_cast<const char*>(buffer + prefix_sum[index]), field_type::field_size);
    } else if constexpr (is_field_with_runtime_size_v<field_type>) {
      field.read(reinterpret_cast<const char*>(buffer + prefix_sum[index]), field_list[field_type::field_accessor]);
    }
    ++index;
  }(static_cast<fields&>(field_list)), ...);
}

template <typename... fields>
constexpr void struct_cast(struct_field_list<fields...>& field_list, std::ifstream& stream) {
  ([&](auto& field) {
    using field_type = std::decay_t<decltype(field)>;
    if constexpr (is_struct_field_list_v<extract_type_from_field_v<field_type>>) {
      struct_cast(field.value, stream);
    } else if constexpr (is_field_v<field_type>) {
      field.read(stream, field_type::field_size);
    } else if constexpr (is_field_with_runtime_size_v<field_type>) {
      field.read(stream, field_list[field_type::field_accessor]);
    }

  }(static_cast<fields&>(field_list)), ...);
}

#endif // _CAST_HPP_
