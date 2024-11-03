#ifndef _FIXED_STR_LIST_HPP_
#define _FIXED_STR_LIST_HPP_

#include "fixed_string.hpp"


template <fixed_string... fs>
struct fixed_string_list {};

template <fixed_string... fs>
using with_fields = fixed_string_list<fs...>;

template <typename T>
struct is_field_name_list;

template <typename T>
struct is_field_name_list {
  static constexpr bool res = false;
};

template <fixed_string... fs>
struct is_field_name_list<with_fields<fs...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_field_name_list_v = is_field_name_list<T>::res;


struct empty_list{};
struct not_a_list{};
struct out_of_bound{};


template <typename T>
struct size;

template <>
struct size<fixed_string_list<>> {
  static constexpr std::size_t N = 0;
};

template <fixed_string head, fixed_string... tail>
struct size<fixed_string_list<head, tail...>> {
  static constexpr std::size_t N = 1 + size<fixed_string_list<tail...>>::N;
};

template <typename T>
inline constexpr std::size_t size_v = size<T>::N;

template <typename T>
struct front;

template <fixed_string head, fixed_string... tail>
struct front<fixed_string_list<head, tail...>> {
  static constexpr auto string = head;
};

template <typename T>
inline constexpr auto front_t = front<T>::string;

template <std::size_t idx, std::size_t key, std::size_t count, typename T>
struct get;

template <std::size_t key, std::size_t count, fixed_string head, fixed_string... tail>
struct get<key, key, count, fixed_string_list<head, tail...>> {
  static constexpr auto string = head;
};

template <std::size_t idx, std::size_t key, std::size_t count, fixed_string head, fixed_string... tail>
struct get<idx, key, count, fixed_string_list<head, tail...>> {
  static constexpr auto string = get<idx + 1, key, count, fixed_string_list<tail...>>::string;
};

template <std::size_t key, typename T>
inline constexpr auto get_t = get<0, key, size_v<T>, T>::string; 

template <std::size_t count, typename T>
struct pop;

template <std::size_t count>
struct pop<count, fixed_string_list<>> {
  using type = fixed_string_list<>;
};

template <std::size_t count, fixed_string head, fixed_string... tail>
struct pop<count, fixed_string_list<head, tail...>> {
  using type = typename pop<count - 1, fixed_string_list<tail...>>::type;
};

template <fixed_string head, fixed_string... tail>
struct pop<0, fixed_string_list<head, tail...>> {
  using type = fixed_string_list<head, tail...>;
};

template <std::size_t count, typename T>
using pop_t = typename pop<count, T>::type;

template <typename T>
concept field_name_list = is_field_name_list_v<T>;


using typelist_ex = fixed_string_list<"a", "b", "c", "d">;
inline constexpr auto idx_list = std::make_integer_sequence<std::size_t, size_v<typelist_ex>>{};
static_assert(fixed_string("a") == front_t<typelist_ex>);
static_assert(std::is_same_v<fixed_string_list<"a", "b", "c", "d">, pop_t<0, typelist_ex>>);
static_assert(std::is_same_v<fixed_string_list<"b", "c", "d">, pop_t<1, typelist_ex>>);
static_assert(std::is_same_v<fixed_string_list<"c", "d">, pop_t<2, typelist_ex>>);
static_assert(fixed_string("c") == front_t<pop_t<2, typelist_ex>>);
static_assert(fixed_string("c") == get_t<2, typelist_ex>);
static_assert(size_v<typelist_ex> == 4);



#endif // _FIXED_STR_LIST_HPP_
