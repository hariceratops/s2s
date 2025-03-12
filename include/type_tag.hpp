#ifndef _TYPE_TAG_HPP_
#define _TYPE_TAG_HPP_

#include "sc_type_traits.hpp"
#include "struct_field_list_base.hpp"
#include "field_size.hpp"


template <typename T, typename size>
concept variable_buffer_type_tag_like = 
  (variable_sized_buffer_like<T> || vector_of_records_like<T>) &&
  (variable_size_like<size>);

template <typename T, typename size>
concept fixed_buffer_type_tag_like = 
  (fixed_buffer_like<T> || array_of_records_like<T>) &&
  (fixed_size_like<size>);

// todo is this required
// todo constraint T and size
template <trivial T, fixed_size_like S>
struct trivial_tag {
  using type = T;
  using size = S;
};

template <field_list_like T>
struct struct_tag {
  using type = T;
  using size = field_size<size_dont_care>;
};

template <typename T, typename S> 
  requires fixed_buffer_type_tag_like<T, S>
struct fixed_buffer_tag {
  using type = T;
  using size = S;
};

template <typename T, typename S> 
  requires variable_buffer_type_tag_like<T, S>
struct variable_buffer_tag {
  using type = T;
  using size = S;
};

template <typename T>
struct is_type_tag;

template <typename T, typename size>
struct is_type_tag<trivial_tag<T, size>> {
  static constexpr bool res = true;
};

template <typename T, typename size>
struct is_type_tag<fixed_buffer_tag<T, size>> {
  static constexpr bool res = true;
};

template <typename T, typename size>
struct is_type_tag<variable_buffer_tag<T, size>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_type_tag<struct_tag<T>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_type_tag {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_type_tag_v = is_type_tag<T>::res;

template <typename T>
concept type_tag_like = is_type_tag_v<T>;


#endif // _TYPE_TAG_HPP_
