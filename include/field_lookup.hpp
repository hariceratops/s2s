#ifndef _FIELD_LOOKUP_HPP_
#define _FIELD_LOOKUP_HPP_


#include "typelist.hpp"
#include "fixed_string.hpp"
#include "field.hpp"


struct field_lookup_failed {};

template <typename ls, fixed_string id>
struct field_lookup;

template <fixed_string id, typename T, std::size_t size, typename... rest>
struct field_lookup<field_list<field<id, T, size>, rest...>, id> {
  using type = field<id, T, size>;
};

template <fixed_string id, typename T, typename... rest>
struct field_lookup<field_list<struct_field<id, T>, rest...>, id> {
  using type = struct_field<id, T>;
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
