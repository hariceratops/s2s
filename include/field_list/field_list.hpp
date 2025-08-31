#ifndef _FIELD_LIST_HPP_
#define _FIELD_LIST_HPP_


#include "../field/field_type_info.hpp"
#include "field_list_base.hpp"
#include "field_list_metadata.hpp"
#include "../lib/metaprog/mp.hpp"


namespace s2s {

template <auto list_metadata, typename... fields>
struct struct_field_list_impl : struct_field_list_base, fields... {

  struct_field_list_impl() = default;

  // todo move as_sv to common place
  template <
    typename field_accessor, 
    auto field_lookup_res = lookup_field<list_metadata>(as_sv(field_accessor::field_id))
  >
    requires (field_lookup_res.has_value)
  constexpr auto& operator[](field_accessor)  {
    using field_type_ref = meta::type_of<field_lookup_res->id>&;
    return static_cast<field_type_ref>(*this).value;
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
