#ifndef _FIELD_LIST_HPP_
#define _FIELD_LIST_HPP_


#include "../field/field_type_info.hpp"
#include "field_list_base.hpp"
#include "field_list_metadata.hpp"
#include "../lib/metaprog/mp.hpp"


namespace s2s {

// A field whose value the write path derives from other fields. The name
// carries the reason into the diagnostic when an assignment is rejected.
template <typename field_accessor, auto list_metadata>
concept field_is_derived_from_other_fields =
  is_derived_field<list_metadata>(as_sv(field_accessor::field_id));

template <auto list_metadata, typename... fields>
struct struct_field_list_impl : struct_field_list_base, fields... {

  struct_field_list_impl() = default;

  // todo move as_sv to common place
  template <
    typename field_accessor,
    auto field_lookup_res = lookup_field<list_metadata>(as_sv(field_accessor::field_id))
  >
    requires (field_lookup_res.has_value) &&
             (!field_is_derived_from_other_fields<field_accessor, list_metadata>)
  constexpr auto& operator[](field_accessor)  {
    using field_type_ref = meta::type_of<field_lookup_res->id>&;
    return static_cast<field_type_ref>(*this).value;
  }

  // Derived fields stay readable on a non-const object but hand back a const
  // reference, so an attempted assignment fails as assign-to-const rather than
  // as a wall of unsatisfied-constraint output from no viable overload.
  template <
    typename field_accessor,
    auto field_lookup_res = lookup_field<list_metadata>(as_sv(field_accessor::field_id))
  >
    requires (field_lookup_res.has_value) &&
             field_is_derived_from_other_fields<field_accessor, list_metadata>
  constexpr const auto& operator[](field_accessor) {
    using field_type_cref = const meta::type_of<field_lookup_res->id>&;
    return static_cast<field_type_cref>(*this).value;
  }

  template <
    typename field_accessor,
    auto field_lookup_res = lookup_field<list_metadata>(as_sv(field_accessor::field_id))
  >
    requires (field_lookup_res.has_value)
  constexpr const auto& operator[](field_accessor) const {
    using field_type_cref = const meta::type_of<field_lookup_res->id>&;
    return static_cast<field_type_cref>(*this).value;
  }
};
} /* namespace s2s */


#endif // _FIELD_LIST_HPP_
