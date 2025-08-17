#ifndef _FIELD_SIZE_DEDUCE_HPP_
#define _FIELD_SIZE_DEDUCE_HPP_


#include "../field/field_accessor.hpp"
#include "../field_list/field_list.hpp"
#include "../field_size/field_size.hpp"
#include "../field_compute/computation_from_fields.hpp"
#include "../field_compute/computation_from_fields_impl.hpp"


namespace s2s {
template <typename T>
struct deduce_field_size;

template <fixed_string id>
struct deduce_field_size<field_size<field_accessor<id>>> {
  using field_size_type = field_accessor<id>;
  
  template <typename... fields>
  constexpr auto operator()(const struct_field_list_impl<fields...>& struct_fields) -> std::size_t {
    return struct_fields[field_size_type{}];
  }
};

template <auto callable, field_name_list req_fields>
struct deduce_field_size<field_size<size_from_fields<callable, req_fields>>> {
  using field_size_type = compute<callable, std::size_t, req_fields>;
  template <typename... fields>
  constexpr auto operator()(const struct_field_list_impl<fields...>& struct_fields) -> std::size_t {
    return compute_impl<field_size_type>{}(struct_fields);
  }
};
} /* namespace s2s */


#endif // _FIELD_SIZE_DEDUCE_HPP_
