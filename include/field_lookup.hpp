#ifndef _FIELD_LOOKUP_HPP_
#define _FIELD_LOOKUP_HPP_


#include "typelist.hpp"
#include "fixed_string.hpp"
#include "field.hpp"


struct field_lookup_failed {};

template <typename ls, fixed_string id>
struct field_lookup;

template <fixed_string id, 
          typename T, 
          typename size, 
          auto field_constraint, 
          typename present_only_if, 
          typename type_deducer, 
          typename... rest>
struct field_lookup<field_list<field<id, T, size, field_constraint, present_only_if, type_deducer>, rest...>, id> {
  using type = field<id, T, size, field_constraint, present_only_if, type_deducer>;
};

template <fixed_string id, typename head, typename... rest>
struct field_lookup<field_list<head, rest...>, id> {
  using type = typename field_lookup<field_list<rest...>, id>::type;
};

template <fixed_string id>
struct field_lookup<field_list<>, id> {
  using type = field_lookup_failed;
};

template <typename ls, fixed_string id>
using field_lookup_v = typename field_lookup<ls, id>::type;


#endif // _FIELD_LOOKUP_HPP_
