#ifndef _COMPUTATION_FROM_FIELDS_IMPL_HPP_
#define _COMPUTATION_FROM_FIELDS_IMPL_HPP_

#include <type_traits>
#include "../lib/metaprog/fixed_string_list.hpp"
#include "../field/field_accessor.hpp"
#include "../field_list/field_list.hpp"
#include "computation_from_fields.hpp"


using namespace s2s_literals;

namespace s2s {

template <typename metadata, typename... fields>
struct struct_field_list_impl;


template <auto callable, typename return_type, typename struct_field_list_t, field_name_list field_list>
struct is_invocable;

template <auto callable, 
          typename return_type, 
          typename struct_field_list_t, 
          fixed_string... req_fields>
struct is_invocable<callable, 
                    return_type, 
                    struct_field_list_t, 
                    fixed_string_list<req_fields...>> {
  static constexpr bool res = 
      std::is_invocable_r_v<
        return_type, 
        decltype(callable),
        decltype(struct_field_list_t{}[field_accessor<req_fields>{}])...
      >;
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
    req_fields
  >::res;


// template <auto callable, typename R, field_name_list Fs>
template <typename T>
struct compute_impl;

// todo: static_vector over fixed_string list?
template <auto callable, typename R, fixed_string... req_fields>
struct compute_impl<compute<callable, R, fixed_string_list<req_fields...>>>{
  template <typename... fields>
    requires (can_eval_R_from_fields<
                callable, 
                R,
                struct_field_list_impl<fields...>,
                fixed_string_list<req_fields...>>)
  constexpr auto operator()(const struct_field_list_impl<fields...>& flist) const -> R {
    return callable(flist[field_accessor<req_fields>{}]...);
  }
};
} /* namespace s2s */

#endif // _COMPUTATION_FROM_FIELDS_IMPL_HPP_
