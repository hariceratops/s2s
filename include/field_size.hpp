#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#include "field_accessor.hpp"
#include "fixed_string_list.hpp"
#include "typelist.hpp"

namespace s2s {
template <typename size_type>
struct field_size;

template <typename size_type>
struct field_size {
  using size_type_t = size_type;
};

struct size_dont_care {};

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto count = N;
};

template <fixed_string id>
using len_from_field = field_accessor<id>;

template <auto callable, field_name_list req_fields>
struct size_from_fields;

// todo constraint for callable
template <auto callable, field_name_list req_fields>
struct size_from_fields {
  static constexpr auto f = callable;
  static constexpr auto req_field_list = req_fields{};
};

template <auto callable, field_name_list ids>
using len_from_fields = size_from_fields<callable, ids>;

// todo size type for holding multiple sizes in case of union fields
template <typename... size_type>
struct size_choices;

template <typename... size_type>
struct size_choices {
  using choices = typelist::list<size_type...>;
  static auto constexpr num_of_choices = sizeof...(size_type);
};

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_fixed_size;

template <std::size_t N>
struct is_fixed_size<field_size<fixed<N>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_fixed_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_fixed_size_v = is_fixed_size<T>::res;

template <typename T>
struct is_variable_size;

template <typename T>
struct is_variable_size {
  static constexpr bool res = false;
};

template <fixed_string id>
struct is_variable_size<field_size<len_from_field<id>>> {
  static constexpr bool res = true;
};

template <auto callable, field_name_list ids>
struct is_variable_size<field_size<len_from_fields<callable, ids>>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variable_size_v = is_variable_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept fixed_size_like = is_fixed_size_v<T>;

template <typename T>
concept variable_size_like = is_variable_size_v<T>;

template <typename T>
struct is_selectable_size;

template <typename T>
concept atomic_size = fixed_size_like<T> || variable_size_like<T>;

template <atomic_size... size_type>
struct is_selectable_size<field_size<size_choices<size_type...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_fixed_size<T>::res;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = fixed_size_like<T>    ||
                       variable_size_like<T> ||
                       selectable_size_like<T>;

template <typename T>
struct is_size_dont_care;

template <>
struct is_size_dont_care<field_size<size_dont_care>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_size_dont_care {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_size_dont_care_v = is_size_dont_care<T>::res;

template <typename T>
concept size_dont_care_like = is_size_dont_care_v<T>;

} /* namespace s2s */


#endif // _FIELD_SIZE_HPP_
