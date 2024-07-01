#ifndef _TYPELIST_HPP_
#define _TYPELIST_HPP_


template <typename... ts>
struct field_list{};

namespace typelist {
template <typename... ts>
struct typelist;

template <typename... ts>
struct typelist{};

template <>
struct typelist<>{};

template <typename... ts>
struct any_of;

template <typename... ts>
struct any_of {};

template <typename t>
struct any_of<typelist<>, t> { static constexpr bool res = false; };

template <typename t, typename... rest>
struct any_of<typelist<t, rest...>, t> { static constexpr bool res = true; };

template <typename t, typename u, typename... rest>
struct any_of<typelist<u, rest...>, t> { static constexpr bool res = false || any_of<typelist<rest...>, t>::res; };

template <typename typelist, typename type>
inline constexpr bool any_of_v = any_of<typelist, type>::res;

static_assert(any_of_v<typelist<int, float, float>, int>);
static_assert(any_of_v<typelist<float, int, float, float>, int>);
static_assert(!any_of_v<typelist<int, int, int>, float>);
static_assert(!any_of_v<typelist<>, float>);
} // namespace typelist



#endif // _TYPELIST_HPP_
