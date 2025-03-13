#ifndef _TYPE_TAG_HPP_
#define _TYPE_TAG_HPP_

#include "sc_type_traits.hpp"
#include "struct_field_list_base.hpp"
#include "field_size.hpp"
#include "size_deduce.hpp"


// todo is this required
// todo constraint T and size
template <trivial T, fixed_size_like S>
  requires (deduce_field_size<S>{}() <= sizeof(T))
struct trivial_tag {
  using type = T;
  using size = S;
};

template <field_list_like T>
struct struct_tag {
  using type = T;
  using size = field_size<size_dont_care>;
};

// todo how to handle array of array
template <trivial T, std::size_t N> 
struct fixed_buffer_tag {
  using type = std::array<T, N>;
  using size = field_size<fixed<N * sizeof(T)>>;
};

template <std::size_t N> 
struct fixed_string_tag {
  using type = fixed_string<N>;
  using size = field_size<fixed<N + 1>>;
};

template <trivial T, variable_size_like S> 
struct variable_buffer_tag {
  using type = std::vector<T>;
  using size = S;
};

template <variable_size_like S> 
struct variable_string_tag {
  using type = std::string;
  using size = S;
};

template <field_list_like T, std::size_t N>
struct array_of_record_tag {
  using type = std::array<T, N>;
  using size = field_size<size_dont_care>;
};

template <field_list_like T, variable_size_like S>
struct vector_of_record_tag {
  using type = std::vector<T>;
  using size = S;
};

template <typename T>
struct is_type_tag;

template <typename T, typename size>
struct is_type_tag<trivial_tag<T, size>> {
  static constexpr bool res = true;
};

template <typename T, std::size_t size>
struct is_type_tag<fixed_buffer_tag<T, size>> {
  static constexpr bool res = true;
};

template <std::size_t size>
struct is_type_tag<fixed_string_tag<size>> {
  static constexpr bool res = true;
};

template <typename T, typename size>
struct is_type_tag<variable_buffer_tag<T, size>> {
  static constexpr bool res = true;
};

template <typename size>
struct is_type_tag<variable_string_tag<size>> {
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
