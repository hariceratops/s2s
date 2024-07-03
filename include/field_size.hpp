#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#include "sc_type_traits.hpp"

template <auto S, typename size_type = decltype(S)> 
  requires fixed_string_like<size_type> || 
           unsigned_integral<size_type> 
struct field_size {
  static auto constexpr size = S;
};


template <typename T>
struct is_field_size;

template <auto size>
struct is_field_size<field_size<size>> {
  static constexpr bool is_size = true;
};

// todo: make this working
// template <std::size_t N, auto s = fixed_string<N>{}>
// struct is_field_size<field_size<s>> {
//   static constexpr bool is_size = true;
// };

template <typename T>
struct is_field_size {
  static constexpr bool is_size = false;
};

template <typename T>
inline constexpr bool is_field_size_v = is_field_size<T>::is_size;

template <typename T>
concept field_size_like = is_field_size_v<T>;

template <std::size_t N>
constexpr auto from_field(const char (&str)[N]) {
  return field_size<fixed_string(str)>{};
}

static_assert(field_size<4u>::size == 4);
static_assert(field_size_like<field_size<4u>>);
static_assert(field_size_like<field_size<fixed_string("GIF")>>);

#endif // _FIELD_SIZE_HPP_
