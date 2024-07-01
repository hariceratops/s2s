#ifndef _CAST_HPP_
#define _CAST_HPP_


#include "struct_field_list.hpp"

struct not_a_field;

template <typename T>
struct extract_type_from_field;

template <fixed_string id, typename field_type, std::size_t size>
struct extract_type_from_field<field<id, field_type, size>> {
  using type = field_type;
};

template <fixed_string id, typename field_type>
struct extract_type_from_field<struct_field<id, field_type>> {
  using type = field_type;
};

template <typename T>
struct extract_type_from_field {
  using type = not_a_field;
};

template <typename T>
using extract_type_from_field_v = typename extract_type_from_field<T>::type;

static_assert(std::is_same_v<extract_type_from_field_v<field<"x", int, 4>>, int>);
static_assert(std::is_same_v<extract_type_from_field_v<field<"x", float, 4>>, float>);
static_assert(std::is_same_v<extract_type_from_field_v<std::array<char, 10>>, not_a_field>);
static_assert(
  std::is_same_v<extract_type_from_field_v<
    struct_field<"d", struct_field_list<field<"x", int, 4>, field<"y", int, 4>>>>, 
    struct_field_list<field<"x", int, 4>, field<"y", int, 4>>>
  );

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
    } else {
      field.read(reinterpret_cast<const char*>(buffer + prefix_sum[index]), field_type::field_size);
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
    } else {
      field.read(stream, field_type::field_size);
    }
  }(static_cast<fields&>(field_list)), ...);
}

#endif // _CAST_HPP_
