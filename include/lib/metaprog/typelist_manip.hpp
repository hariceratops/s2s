#ifndef _TYPELIST_MANIP_HPP_
#define _TYPELIST_MANIP_HPP_

#include <variant>
#include "typelist.hpp"
#include "fixed_string_list.hpp"


// status: might be deprecated due to value MP

namespace s2s {
// todo typelist to variant?
template <typename T>
struct variant_from_tlist;

template <typename... type>
struct variant_from_tlist<typelist::typelist<type...>> {
  using variant = std::variant<type...>;
  static constexpr std::size_t N = sizeof...(type);
};

template <typename T>
using variant_from_tlist_v = typename variant_from_tlist<T>::variant;

template <typename T>
inline constexpr std::size_t variant_size_n = variant_from_tlist<T>::N;


template <typename T>
struct tl_size;

template <>
struct tl_size<typelist::typelist<>> {
  static constexpr std::size_t N = 0;
};

template <typename head, typename... tail>
struct tl_size<typelist::typelist<head, tail...>> {
  static constexpr std::size_t N = 1 + tl_size<typelist::typelist<tail...>>::N;
};

template <typename T>
inline constexpr std::size_t tl_size_v = tl_size<T>::N;

template <typename T>
struct tl_front;

template <typename head, typename... tail>
struct tl_front<typelist::typelist<head, tail...>> {
  using type = head;
};

template <typename T>
using tl_front_t = typename tl_front<T>::type;

template <std::size_t idx, std::size_t key, std::size_t count, typename T>
struct tl_get;

template <std::size_t key, std::size_t count, typename head, typename... tail>
struct tl_get<key, key, count, typelist::typelist<head, tail...>> {
  using type = head;
};

template <std::size_t idx, std::size_t key, std::size_t count, typename head, typename... tail>
struct tl_get<idx, key, count, typelist::typelist<head, tail...>> {
  using type = typename tl_get<idx + 1, key, count, typelist::typelist<tail...>>::type;
};

template <std::size_t key, typename T>
using tl_get_t = typename tl_get<0, key, tl_size_v<T>, T>::type; 

template <std::size_t count, typename T>
struct tl_pop;

template <std::size_t count>
struct tl_pop<count, typelist::typelist<>> {
  using type = fixed_string_list<>;
};

template <std::size_t count, typename head, typename... tail>
struct tl_pop<count, typelist::typelist<head, tail...>> {
  using type = typename pop<count - 1, typelist::typelist<tail...>>::type;
};

template <typename head, typename... tail>
struct tl_pop<0, typelist::typelist<head, tail...>> {
  using type = typelist::typelist<head, tail...>;
};

template <std::size_t count, typename T>
using tl_pop_t = typename tl_pop<count, T>::type;


template <std::size_t idx, typename T>
struct get_nth_variant_type;

template <std::size_t idx, typename... types>
struct get_nth_variant_type<idx, std::variant<types...>> {
  using type = tl_front_t<tl_pop_t<idx, typelist::typelist<types...>>>;
};

template <std::size_t idx, typename T>
using get_nth_variant_type_t = typename get_nth_variant_type<idx, T>::type;

template <typename... types>
struct variant_to_typelist;

template <typename... types>
struct variant_to_typelist<std::variant<types...>> {
  using type = typelist::typelist<types...>;
  static constexpr std::size_t N = sizeof...(types);
};

template <typename T>
using variant_to_typelist_t = typename variant_to_typelist<T>::type;

template <typename T>
inline constexpr std::size_t variant_size = variant_to_typelist<T>::N;


struct out_of_bounds {};

template <typename T>
struct iseq_pop;

template <>
struct iseq_pop<std::integer_sequence<std::size_t>> {
  using type = out_of_bounds;
};

template <std::size_t head, std::size_t... size_tail>
struct iseq_pop<std::integer_sequence<std::size_t, head, size_tail...>> {
  using type = std::integer_sequence<std::size_t, size_tail...>;
};

template <typename T>
using iseq_pop_t = typename iseq_pop<T>::type;

template <typename T>
struct iseq_front;

template <std::size_t head, std::size_t... tail>
struct iseq_front<std::integer_sequence<std::size_t, head, tail...>> {
  static constexpr std::size_t size = head;
};

template <>
struct iseq_front<std::integer_sequence<std::size_t>> {};

template <typename T>
inline constexpr std::size_t iseq_front_v = iseq_front<T>::size;
} /* namespace s2s */

#endif // _TYPELIST_MANIP_HPP_
