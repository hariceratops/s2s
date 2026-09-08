#ifndef _ANNOUNCING_RECORD_HPP_
#define _ANNOUNCING_RECORD_HPP_


#include "../field/field_traits.hpp"
#include "../order_deduction/order_deduction.hpp"
#include "field_list.hpp"
#include "field_list_metadata.hpp"


namespace s2s {
// The names a byte-order announcement reads. Extracted with the same shape and
// the same helpers extract_type_deduction_dependencies uses, but deliberately
// not through it: that metafunction feeds a dependency table consumed by
// is_dependencies_resolved, which looks every name up in the *declaring* list's
// field table and dereferences the result. An announcement's names are fields
// of the inner record, so routing them there would make every such schema a
// constant-evaluation error.
//
// Existence is also the whole obligation. The ordering half that check performs
// is vacuous on this axis: every field of the announcing record is read before
// resolution runs, so no name can be "not yet read".
//
// Sits above field_list.hpp rather than inside the metadata, because reaching
// the inner record's table means pattern-matching struct_field_list_impl, which
// the metadata header is included by.
template <typename guide>
struct order_deduction_dependencies;

template <fixed_string id, typename _switch>
struct order_deduction_dependencies<order_from<match_field<id>, _switch>> {
  static constexpr auto value = dep_vec(as_sv(id));
};

template <auto callable, typename R, fixed_string... req_fields, typename _switch>
struct order_deduction_dependencies<
  order_from<compute_t<callable, R, fixed_string_list<req_fields...>>, _switch>
>
{
  static constexpr auto value = dep_vec(as_sv(req_fields)...);
};

template <typename... branches>
struct order_deduction_dependencies<order_from<order_if_else<branches...>>> {
  static constexpr dep_vec deps[64] = {dep_vec(extract_req_fields_from_clause_v<branches>)...};
  static constexpr auto flat_values = flatten(deps);
  static constexpr auto value = remove_duplicates(flat_values);
};


template <typename T>
struct field_table_of;

template <auto metadata, typename... fields>
struct field_table_of<struct_field_list_impl<metadata, fields...>> {
  static constexpr auto value = meta::type_of<metadata>::field_table;
};


constexpr auto all_names_present(const field_table_t& table, const dep_vec& names) -> bool {
  for(auto name: names) {
    if(!table[name].has_value())
      return false;
  }
  return true;
}


template <typename T>
struct announcement_names_resolve {
  static constexpr bool res = true;
};

template <fixed_string id, field_list_like T, auto size, auto constraint, auto bound,
          typename guide>
struct announcement_names_resolve<
  order_announcing_field<field<id, T, size, constraint, bound>, guide>
>
{
  static constexpr bool res =
    all_names_present(field_table_of<T>::value, order_deduction_dependencies<guide>::value);
};

template <typename T>
inline constexpr bool announcement_names_resolve_v = announcement_names_resolve<T>::res;
} /* namespace s2s */


#endif // _ANNOUNCING_RECORD_HPP_
