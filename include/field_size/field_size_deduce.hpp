#ifndef _FIELD_SIZE_DEDUCE_HPP_
#define _FIELD_SIZE_DEDUCE_HPP_


#include "../field/field_accessor.hpp"
#include "../field_list/field_list.hpp"
#include "../field_size/field_size.hpp"
#include "../field_size/comptime_field_size_deduce.hpp"
#include "../field_compute/computation_from_fields.hpp"
#include "../field_compute/computation_from_fields_impl.hpp"


namespace s2s {
// Two more partial specializations of the template declared in
// comptime_field_size_deduce.hpp, with argument lists identical to the
// primary's and told apart only by their constraints.
template <auto size>
  requires (variable_size_like<size_type_of<size>> && !is_computed_size_v<size_type_of<size>>)
struct deduce_field_size<size> {
  template <auto metadata, typename... fields>
  constexpr auto operator()(const struct_field_list_impl<metadata, fields...>& struct_fields) const -> std::size_t {
    return struct_fields[size];
  }
};

template <auto size>
  requires is_computed_size_v<size_type_of<size>>
struct deduce_field_size<size> {
  using field_size_type =
    compute<size_type_of<size>::f, std::size_t, std::remove_cvref_t<decltype(size_type_of<size>::req_field_list)>>;

  template <auto metadata, typename... fields>
  constexpr auto operator()(const struct_field_list_impl<metadata, fields...>& struct_fields) const -> std::size_t {
    return compute_impl<field_size_type>{}(struct_fields);
  }
};
} /* namespace s2s */


#endif // _FIELD_SIZE_DEDUCE_HPP_
