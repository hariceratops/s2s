#ifndef _COMPUTATION_FROM_FIELDS_HPP_
#define _COMPUTATION_FROM_FIELDS_HPP_

#include <type_traits>
#include "../field/field_accessor.hpp"
#include "../lib/metaprog/fixed_string_list.hpp"


using namespace s2s_literals;

namespace s2s {
template <auto callable, typename R, field_name_list Fs>
struct compute;

// todo: static_vector over fixed_string list?
template <auto callable, typename R, fixed_string... req_fields>
struct compute<callable, R, fixed_string_list<req_fields...>>{
  static constexpr auto C = callable;
  using return_type = R;
  using in_arg_types = fixed_string_list<req_fields...>;
};

template <auto callable, field_name_list req_fields>
using eval_bool_from_fields = compute<callable, bool, req_fields>;

template <auto callable, field_name_list req_fields>
using predicate = compute<callable, bool, req_fields>;

template <auto callable, field_name_list req_fields>
using eval_size_from_fields = compute<callable, std::size_t, req_fields>;

template <auto callable, field_name_list req_fields>
using parse_if = eval_bool_from_fields<callable, req_fields>;
} /* namespace s2s */

#endif // _COMPUTATION_FROM_FIELDS_HPP_
