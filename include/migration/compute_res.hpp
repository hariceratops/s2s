#ifndef _COMPUTE_RES_
#define _COMPUTE_RES_

#include <type_traits>
#include "../typelist.hpp"
#include "../struct_field_list_base.hpp"
#include <functional>
#include <optional>
#include "../field_accessor.hpp"
#include "field_reader.hpp"


// todo add constriants
template <auto callable, fixed_string... req_fields>
struct compute;

template <auto callable, fixed_string... req_fields>
struct compute<callable, typelist::typelist<req_fields...>> {
  template <field_list_like flist>
  constexpr auto operator()(flist& field_list) {
    return std::invoke(callable, field_list[field_accessor<req_fields::field_id>{}]...);
  }
};

template <typename T>
struct is_compute_like;

template <auto callable, fixed_string... req_fields>
struct is_compute_like<compute<callable, req_fields...>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_compute_like {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_size_compute_like_v = is_compute_like<T>::res;

template <typename T, typename U>
concept size_compute_like = 
  is_size_compute_like_v<T> &&
  requires (T& compute_callable, U& field_list) {
    { std::is_base_of_v<struct_field_list_base, U> };
    { compute_callable(field_list) } -> std::convertible_to<std::size_t>;
  };

template <typename T, typename U>
concept bool_compute_like = 
  is_size_compute_like_v<T> &&
  requires (T& compute_callable, U& field_list) {
    { std::is_base_of_v<struct_field_list_base, U> };
    { compute_callable(field_list) } -> std::convertible_to<bool>;
  };


// todo reader callable struct

template <auto v, typename T>
struct match_case {
  static constexpr auto value = v;
  static constexpr auto reader = 
    [](T& dst, const unsigned char* buffer, std::size_t size_to_read) {
      read(dst, buffer, size_to_read);
    };
};

template <typename... Cases>
struct match_and_read;

template <>
struct match_and_read<> {
  constexpr void operator()(auto&, auto, const unsigned char*, std::size_t) const {
      // No match found, do nothing or handle the case
  }
};

// Recursive case: check head and proceed to tail
template <typename match_case_head, typename... match_case_rest>
struct match_and_read<match_case_head, match_case_rest...> {
  constexpr void operator()(auto& another, auto v, const unsigned char* buffer, std::size_t size_to_read) const {
    (v == match_case_head::value ? match_case_head::reader(another, buffer, size_to_read)
                                 : match_and_read<match_case_rest...>()(another, v, buffer, size_to_read));
  }
};

template <auto callable_ret_bool, typename T>
struct match_case_by_callable {
  static constexpr auto callable = callable_ret_bool;
  constexpr void operator()(T& dst, const unsigned char* buffer, std::size_t size_to_read) {
    read(dst, buffer, size_to_read);
  }
  constexpr void operator()(T& dst, std::ifstream& stream, std::size_t size_to_read) {
    read(dst, stream, size_to_read);
  }
};

template <auto... cases>
struct another_match_and_read;

template <>
struct another_match_and_read<> {
  constexpr void operator()(auto&, auto, const unsigned char*, std::size_t) const {}
};

template <auto callable_head, auto... callable_rest>
struct another_match_and_read<callable_head, callable_rest...> {
};

#endif // _COMPUTE_RES_
