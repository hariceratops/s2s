#ifndef _COMPUTE_RES_
#define _COMPUTE_RES_

#include "typelist.hpp"
#include "field.hpp"
#include "struct_field_list.hpp"
#include <functional>
#include "cast.hpp"


// todo add constriants
template <auto callable, typename... req_fields>
struct compute;

template <auto callable, typename... req_fields>
struct compute<callable, typelist::typelist<req_fields...>> {
  template <typename... fields>
  constexpr auto operator()(struct_field_list<fields...>& field_list) {
    return std::invoke(callable, field_list[field_accessor<req_fields::field_id>{}]...);
  }
};

#endif // _COMPUTE_RES_
