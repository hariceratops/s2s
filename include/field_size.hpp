#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#include "field_accessor.hpp"
#include "fixed_str_list.hpp"
#include "typelist.hpp"


template <typename size_type>
struct field_size;

template <typename size_type>
struct field_size {
  using size_type_t = size_type;
  // static constexpr auto size = size_type{};
};

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto count = N;
};

template <typename T>
struct runtime_size;

template <typename field_accessor>
struct runtime_size {
  static constexpr auto accessor = field_accessor{};
};

template <fixed_string id>
using from_field = runtime_size<field_accessor<id>>;

template <auto callable, field_name_list req_fields>
struct size_from_fields;

template <auto callable, field_name_list req_fields>
struct size_from_fields {
  static constexpr auto f = callable;
  static constexpr auto req_field_list = req_fields{};
};

// todo size type for holding multiple sizes in case of union fields
template <typename... size_type>
struct size_choices;

template <typename... size_type>
struct size_choices {
  using choices = typelist::typelist<size_type...>;
  static auto constexpr num_of_choices = sizeof...(size_type);
};

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_comptime_size;

template <std::size_t N>
struct is_comptime_size<field_size<fixed<N>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_size_v = is_comptime_size<T>::res;

template <typename T>
struct is_runtime_size;

template <typename T>
struct is_runtime_size {
  static constexpr bool res = false;
};

template <auto field_accessor>
struct is_runtime_size<field_size<from_field<field_accessor>>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_runtime_size_v = is_runtime_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept comptime_size_like = is_comptime_size_v<T>;

template <typename T>
concept runtime_size_like = is_runtime_size_v<T>;

template <typename T>
struct is_selectable_size;

template <typename T>
concept atomic_size = comptime_size_like<T> || runtime_size_like<T>;

template <atomic_size... size_type>
struct is_selectable_size<field_size<size_choices<size_type...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_comptime_size<T>::res;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = comptime_size_like<T> ||
                       runtime_size_like<T> ||
                       selectable_size_like<T>;

namespace static_test {
static_assert(is_runtime_size_v<field_size<from_field<"hello">>>);
static_assert(is_comptime_size_v<field_size<fixed<4>>>);
static_assert(!is_comptime_size_v<int>);
static_assert(!is_runtime_size_v<int>);
static_assert(field_size<fixed<6>>::size_type_t::count == 6);
}

#endif // _FIELD_SIZE_HPP_
