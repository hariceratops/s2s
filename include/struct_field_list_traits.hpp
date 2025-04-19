#ifndef _STRUCT_FIELD_LIST_TRAITS_HPP_
#define _STRUCT_FIELD_LIST_TRAITS_HPP_

#include <type_traits>
#include "field_list.hpp"

// Metafunction to check if a type is a struct_field_list
template <typename T>
struct is_struct_field_list : std::false_type {};

template <typename... Entries>
struct is_struct_field_list<struct_field_list<Entries...>> : std::true_type {};

template <typename T>
constexpr bool is_struct_field_list_v = is_struct_field_list<T>::value;

#endif // _STRUCT_FIELD_LIST_TRAITS_HPP_
