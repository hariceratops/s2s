#ifndef _COMPUTATION_FROM_FIELDS_TRAITS_HPP_
#define _COMPUTATION_FROM_FIELDS_TRAITS_HPP_

#include "../field/field_accessor.hpp"
#include "../lib/containers/fixed_string.hpp"
#include "computation_from_fields.hpp"


using namespace s2s_literals;

namespace s2s {
template <typename T>
struct is_compute_like;

template <auto callable, typename R, fixed_string... req_fields>
struct is_compute_like<compute_t<callable, R, fixed_string_list<req_fields...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_compute_like {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_compute_like_v = is_compute_like<T>::res;

template <typename T>
struct is_eval_bool_from_fields;

template <auto callable, fixed_string... req_fields>
struct is_eval_bool_from_fields<compute_t<callable, bool, fixed_string_list<req_fields...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_eval_bool_from_fields {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_eval_bool_from_fields_v = is_eval_bool_from_fields<T>::res;

template <typename T>
struct is_eval_size_from_fields;

template <auto callable, fixed_string... req_fields>
struct is_eval_size_from_fields<compute_t<callable, std::size_t, fixed_string_list<req_fields...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_eval_size_from_fields {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_eval_size_from_fields_v = is_eval_size_from_fields<T>::res;
} /* namespace s2s */

#endif // _COMPUTATION_FROM_FIELDS_TRAITS_HPP_
