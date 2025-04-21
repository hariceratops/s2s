#ifndef _FIELD_LIST_METAFUNCTIONS_HPP_
#define _FIELD_LIST_METAFUNCTIONS_HPP_


#include "typelist.hpp"
#include "fixed_string.hpp"
#include "field.hpp"


namespace s2s {
// Sentinel type for a failed lookup
struct field_lookup_failed {};

// Primary template declaration for field_lookup
template <typename field_list, fixed_string id>
struct field_lookup;

// Success case 1: Match a field with the given id
template <fixed_string id, 
          typename T, 
          typename size_type, 
          auto constraint, 
          typename... rest>
struct field_lookup<
    field_list<field<id, T, size_type, constraint>, rest...>, id
  >
{
  using type = field<id, T, size_type, constraint>;
};

// Success case 2: Match a maybe_field with the given id
template <fixed_string id, 
          typename T, 
          typename size, 
          typename present_only_if, 
          auto constraint, 
          typename optional,
          typename... rest>
struct field_lookup<
    field_list<maybe_field<field<id, T, size, constraint>, present_only_if, optional>, rest...>, id
  > 
{
  using type = 
    maybe_field<
      field<id, T, size, constraint>,
      present_only_if, 
      optional
    >;
};

// Success case 3: Match a union_field with the given id
template <fixed_string id,
          typename type_deducer,
          auto constraint_on_value,
          typename T,
          typename size_type,
          typename variant,
          typename field_choices_t,
          typename... rest>
struct field_lookup<
    field_list<union_field<id, type_deducer, T, size_type, constraint_on_value, variant, field_choices_t>, rest...>, id
  >
{
  using type = union_field<id, type_deducer, T, size_type, constraint_on_value, variant, field_choices_t>;
};

// Recursive case: id does not match the head, continue searching in the rest
template <typename head, typename... rest, fixed_string id>
struct field_lookup<field_list<head, rest...>, id> {
  using type = typename field_lookup<field_list<rest...>, id>::type;
};

// Failure case: Reached the end of the field list without finding a match
template <fixed_string id>
struct field_lookup<field_list<>, id> {
  using type = field_lookup_failed;
};

// Alias for easier use
template <typename field_list_t, fixed_string id>
using field_lookup_v = typename field_lookup<field_list_t, id>::type;
} /* namespace s2s */

#endif // _FIELD_LIST_METAFUNCTIONS_HPP_
