#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#include "field_accessor.hpp"
#include "bytes.hpp"


template <typename size_type>
struct field_size;

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto size = N;
};

// todo size type for holding multiple sizes in case of union fields
template <std::size_t... N>
struct size_choices;

template <std::size_t... N>
struct size_choices {
  static constexpr auto sizes = std::integer_sequence<std::size_t, N...>{};
};

template <typename T>
struct runtime_size;

template <typename field_accessor>
struct runtime_size {
  static constexpr auto accessor = field_accessor{};
};

template <fixed_string id>
using from_field = runtime_size<field_accessor<id>>;

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

template <typename T>
struct is_selectable_size;

template <std::size_t... N>
struct is_selectable_size<field_size<size_choices<N...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_comptime_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept comptime_size_like = is_comptime_size_v<T>;

template <typename T>
concept runtime_size_like = is_runtime_size_v<T>;

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
}

#endif // _FIELD_SIZE_HPP_
