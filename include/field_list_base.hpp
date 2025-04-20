#ifndef _FIELD_LIST_BASE_HPP_
#define _FIELD_LIST_BASE_HPP_

#include <type_traits>

namespace s2s {
struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;
} /* namespace s2s */

#endif // _FIELD_LIST_BASE_HPP_
