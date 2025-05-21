#ifndef _TYPE_DEDUCTION_TAGS_
#define _TYPE_DEDUCTION_TAGS_

#include "../lib/s2s_traits/type_traits.hpp"
#include "../field_list/field_list_base.hpp"
#include "../field_size/field_size.hpp"
#include "../field_size/field_size_deduce.hpp"


namespace s2s {
// todo is this required
template <trivial T, fixed_size_like S>
  requires (deduce_field_size<S>{}() <= sizeof(T))
struct as_trivial {
  using type = T;
  using size = S;
};

template <field_list_like T>
struct as_struct {
  using type = T;
  using size = field_size<size_dont_care>;
};

// todo how to handle array of array
template <trivial T, std::size_t N> 
struct as_fixed_arr {
  using type = std::array<T, N>;
  using size = field_size<fixed<N * sizeof(T)>>;
};

template <std::size_t N> 
struct as_fixed_string {
  using type = fixed_string<N>;
  using size = field_size<fixed<N + 1>>;
};

template <trivial T, variable_size_like S> 
struct as_vec {
  using type = std::vector<T>;
  using size = S;
};

template <variable_size_like S> 
struct as_string {
  using type = std::string;
  using size = S;
};

template <field_list_like T, std::size_t N>
struct as_arr_of_records {
  using type = std::array<T, N>;
  using size = field_size<size_dont_care>;
};

template <field_list_like T, variable_size_like S>
struct as_vec_of_records {
  using type = std::vector<T>;
  using size = S;
};

template <typename T>
struct is_type_tag;

template <typename T, typename size>
struct is_type_tag<as_trivial<T, size>> {
  static constexpr bool res = true;
};

template <typename T, std::size_t size>
struct is_type_tag<as_fixed_arr<T, size>> {
  static constexpr bool res = true;
};

template <std::size_t size>
struct is_type_tag<as_fixed_string<size>> {
  static constexpr bool res = true;
};

template <typename T, typename size>
struct is_type_tag<as_vec<T, size>> {
  static constexpr bool res = true;
};

template <typename size>
struct is_type_tag<as_string<size>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_type_tag<as_struct<T>> {
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
} /* namespace s2s */

#endif // _TYPE_DEDUCTION_TAGS_
