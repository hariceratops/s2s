#ifndef _STRUCT_FIELD_LIST_BASE_HPP_
#define _STRUCT_FIELD_LIST_BASE_HPP_

#include <type_traits>

struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;

#endif // _STRUCT_FIELD_LIST_BASE_HPP_
