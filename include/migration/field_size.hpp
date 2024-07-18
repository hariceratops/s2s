#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#include "../sc_type_traits.hpp"
#include "../field_accessor.hpp"


// todo enforce concept constraints for field_size
template <std::size_t N>
struct field_size {
  static constexpr auto size = N;
};

template <fixed_string id>
using from_field = field_accessor<id>;

template <typename field_accessor>
struct runtime_size {
  // static constexpr auto accessor = field_accessor::field_id;
  static constexpr auto accessor = field_accessor{};
};

template <typename T>
struct is_comptime_size;

template <std::size_t N>
struct is_comptime_size<field_size<N>> {
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

template <typename field_accessor>
struct is_runtime_size<runtime_size<field_accessor>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_runtime_size_v = is_runtime_size<T>::res;

template <typename T>
concept comptime_size_like = is_comptime_size_v<T>;

template <typename T>
concept runtime_size_like = is_runtime_size_v<T>;

namespace static_test {
static_assert(is_runtime_size_v<runtime_size<field_accessor<"hello">>>);
static_assert(is_comptime_size_v<field_size<4>>);
static_assert(!is_comptime_size_v<int>);
static_assert(!is_runtime_size_v<int>);
}

#endif // _FIELD_SIZE_HPP_
