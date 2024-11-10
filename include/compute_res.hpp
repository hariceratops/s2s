#ifndef _COMPUTE_RES_
#define _COMPUTE_RES_

#include <type_traits>
#include <functional>
#include "field_accessor.hpp"
#include "field_list.hpp"
#include "fixed_str_list.hpp"

// todo add constriants
template <auto callable, typename R, field_name_list fstr_list>
struct compute;


template <auto callable, typename R, typename struct_fields, typename field_list, typename indices>
struct is_invocable;

template <auto callable, 
          typename R, 
          typename struct_fields, 
          fixed_string_like req_field_list, 
          std::size_t... idx>
struct is_invocable<callable, 
                    R, 
                    struct_fields, 
                    req_field_list, 
                    std::index_sequence<idx...>> {
  static constexpr bool res = 
      std::is_invocable_r<
        R, 
        decltype(callable),
        decltype(struct_fields{}[field_accessor<front_t<pop_t<idx, req_field_list>>>{}])...
      >::value; 
};

template <auto Func, 
          typename R, 
          typename struct_fields, 
          typename req_fields>
concept can_eval_R_from_fields = 
  is_invocable<
    Func, 
    R, 
    struct_fields, 
    req_fields,
    std::make_integer_sequence<std::size_t, size_v<req_fields>>
  >::res;

// todo: expression evaluation requested by user shall not be empty but default to empty by library
// todo bring invocable compatibility at type level for strong type guarantee
// todo simplified concept or requires clause
// todo should cv qualification be removed
// todo role of with_fields and variadic arguments must be reversed, can typelist + idx be used?
template <auto callable, typename R, field_name_list req_fields>
struct compute {
  template <typename... fields, std::size_t... idx>
  constexpr auto invoke_impl(struct_field_list<fields...>& flist, std::index_sequence<idx...>) {
    return std::invoke(callable, flist[field_accessor<front_t<pop_t<idx, req_fields>>>{}]...);
  }

  template <typename... fields>
    requires (can_eval_R_from_fields<
                callable, 
                R,
                struct_field_list<fields...>,
                req_fields>)
  constexpr auto operator()(struct_field_list<fields...>& flist) {
    return invoke_impl(flist, std::make_integer_sequence<std::size_t, size_v<req_fields>>{});
  }
};

template <auto callable, field_name_list req_fields>
using eval_bool_from_fields = compute<callable, bool, req_fields>;
template <auto callable, field_name_list req_fields>
using eval_size_from_fields = compute<callable, std::size_t, req_fields>;
template <auto callable, field_name_list req_fields>
using parse_if = eval_bool_from_fields<callable, req_fields>;

template <typename T>
struct is_compute_like;

template <auto callable, typename R, fixed_string... req_fields>
struct is_compute_like<compute<callable, R, with_fields<req_fields...>>> {
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
struct is_eval_bool_from_fields<compute<callable, bool, with_fields<req_fields...>>> {
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
struct is_eval_size_from_fields<compute<callable, std::size_t, with_fields<req_fields...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_eval_size_from_fields {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_eval_size_from_fields_v = is_eval_size_from_fields<T>::res;


#endif // _COMPUTE_RES_
