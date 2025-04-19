#ifndef _LIST_HPP_
#define _LIST_HPP_

// todo abstract typelist and non type list to avoid code repetition

#include <type_traits>

template <typename... ts>
struct type_list;

template <auto... e>
struct non_type_list;

struct null {};

template <typename... ts>
struct typelist;

template <typename... ts>
struct typelist{};

template <>
struct typelist<>{};

template <typename... ts>
struct any_of_typelist;

template <typename... ts>
struct any_of_typelist {};

template <typename t>
struct any_of_typelist<typelist<>, t> { static constexpr bool res = false; };

template <typename t, typename... rest>
struct any_of_typelist<typelist<t, rest...>, t> { static constexpr bool res = true; };

template <typename t, typename u, typename... rest>
struct any_of_typelist<typelist<u, rest...>, t> { static constexpr bool res = false || any_of_typelist<typelist<rest...>, t>::res; };

template <typename typelist, typename type>
inline constexpr bool any_of_v = any_of_typelist<typelist, type>::res;

template <typename... ts>
struct all_are_same;

template <>
struct all_are_same<typelist<>> {
  static constexpr auto all_same = true;
};

template <typename T>
struct all_are_same<typelist<T>> {
  static constexpr auto all_same = true;
};

template <typename T, typename U, typename... rest>
struct all_are_same<typelist<T, U, rest...>> {
  static constexpr auto all_same = std::is_same_v<T, U> && all_are_same<typelist<U, rest...>>::all_same;
};

template <typename T, typename... rest>
struct all_are_same<typelist<T, rest...>> {
  static constexpr auto all_same = false;
};

template <typename tlist>
inline constexpr bool all_are_same_v = all_are_same<tlist>::all_same;

template <typename... ts>
struct front;

template <typename t, typename... ts>
struct front<typelist<t, ts...>> {
  using front_t = t;
};

template <>
struct front<typelist<>> {
  using front_t = null;
};

template <typename tlist>
using front_t = typename front<tlist>::front_t;

#endif // _LIST_HPP_
