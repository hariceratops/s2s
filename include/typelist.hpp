#ifndef _TYPELIST_HPP_
#define _TYPELIST_HPP_


// status: might be deprecated due to value MP

#include <string>
#include <type_traits>


namespace s2s {
namespace typelist {
struct null {};

template <typename... ts>
struct list;

template <typename... ts>
struct list{};

template <>
struct list<>{};

template <typename... ts>
struct any_of;

template <typename... ts>
struct any_of {};

template <typename t>
struct any_of<list<>, t> { static constexpr bool res = false; };

template <typename t, typename... rest>
struct any_of<list<t, rest...>, t> { static constexpr bool res = true; };

template <typename t, typename u, typename... rest>
struct any_of<list<u, rest...>, t> { static constexpr bool res = false || any_of<list<rest...>, t>::res; };

template <typename list, typename type>
inline constexpr bool any_of_v = any_of<list, type>::res;

template <typename... ts>
struct all_are_same;

template <>
struct all_are_same<list<>> {
  static constexpr auto all_same = true;
};

template <typename T>
struct all_are_same<list<T>> {
  static constexpr auto all_same = true;
};

template <typename T, typename U, typename... rest>
struct all_are_same<list<T, U, rest...>> {
  static constexpr auto all_same = std::is_same_v<T, U> && all_are_same<list<U, rest...>>::all_same;
};

template <typename T, typename... rest>
struct all_are_same<list<T, rest...>> {
  static constexpr auto all_same = false;
};

template <typename L>
inline constexpr bool all_are_same_v = all_are_same<L>::all_same;

template <typename... ts>
struct front;

template <typename t, typename... ts>
struct front<list<t, ts...>> {
  using front_t = t;
};

template <>
struct front<list<>> {
  using front_t = null;
};

template <typename L>
using front_t = typename front<L>::front_t;

} // namespace list
} /* namespace s2s */

#endif // _TYPELIST_HPP_
