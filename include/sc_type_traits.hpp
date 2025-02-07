#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <vector>
#include <variant>
#include <optional>
#include "fixed_string.hpp"
#include "struct_field_list_base.hpp"

// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
struct is_fixed_string;

template <std::size_t N>
struct is_fixed_string<fixed_string<N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_string {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_string_v = is_fixed_string<T>::is_same;

template <typename T>
concept fixed_string_like = is_fixed_string_v<T>;

template <typename T>
struct is_fixed_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_fixed_array<T>::is_same)
struct is_fixed_array<std::array<T, N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_array_v = is_fixed_array<T>::is_same;

template <typename T>
struct is_c_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_c_array<T>::is_same)
struct is_c_array<T[N]> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_c_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_c_array_v = is_c_array<T>::is_same;

// fixed_buffer_like concept
// todo constrain to array of primitives 
// todo check if array of records and arrays are possible for implementation
// todo check if md string is ok
template <typename T>
concept fixed_buffer_like = 
  is_fixed_array_v<T> ||
  is_c_array_v<T> ||
  is_fixed_string_v<T>;

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
struct is_vector_like;

// vector of vectors or vector of arrays?
template <typename T>
  requires (field_list_like<T> || arithmetic<T> || is_fixed_array<T>::is_same)
struct is_vector_like<std::vector<T>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_vector_like {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_vector_v = is_vector_like<T>::res;

template <typename T>
concept vector_like = is_vector_v<T>;

template <typename T>
struct is_string_like;

// vector of vectors or vector of arrays?
template <>
struct is_string_like<std::string> {
  static constexpr bool res = true;
};

template <typename T>
struct is_string_like {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_string_v = is_string_like<T>::res;

template <typename T>
concept string_like = is_string_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

// template <typename T>
// concept fixed_buffer_containable = field_containable<T> || field_list_like

#endif // _SC_META_HPP_
