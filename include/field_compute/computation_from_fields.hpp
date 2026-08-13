#ifndef _COMPUTATION_FROM_FIELDS_HPP_
#define _COMPUTATION_FROM_FIELDS_HPP_


#include "../lib/metaprog/fixed_string_list.hpp"


namespace s2s {
// compute_t keeps a field_name_list *type* so that every internal pattern
// match on it stays a plain partial specialization; `compute` is the spelling a
// schema writes, and normalizes bare ids into that type.
template <auto callable, typename R, field_name_list Fs>
struct compute_t;

template <auto callable, typename R, fixed_string... req_fields>
struct compute_t<callable, R, fixed_string_list<req_fields...>>{
};

template <auto callable, typename R, fixed_string... ids>
using compute = compute_t<callable, R, field_names_of<ids...>>;

template <auto callable, fixed_string... ids>
using eval_bool_from_fields = compute_t<callable, bool, field_names_of<ids...>>;

template <auto callable, fixed_string... ids>
using predicate = compute_t<callable, bool, field_names_of<ids...>>;

template <auto callable, fixed_string... ids>
using eval_size_from_fields = compute_t<callable, std::size_t, field_names_of<ids...>>;

template <auto callable, fixed_string... ids>
using parse_if = eval_bool_from_fields<callable, ids...>;
} /* namespace s2s */

#endif // _COMPUTATION_FROM_FIELDS_HPP_
