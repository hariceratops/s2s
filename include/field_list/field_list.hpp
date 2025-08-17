#ifndef _FIELD_LIST_HPP_
#define _FIELD_LIST_HPP_

// status: rename to impl

#include "../field/field.hpp"
#include "../field/field_type_info.hpp"
#include "../lib/metaprog/typelist.hpp"
#include "../lib/containers/fixed_string.hpp"
#include "field_list_base.hpp"
#include "../field/field_traits.hpp"
// #include "field_list_metadata.hpp"
#include "field_list_lookup.hpp"
#include "../lib/metaprog/mp.hpp"


namespace s2s {

template <typename list_metadata>
constexpr auto lookup_field(std::string_view field_name) -> std::optional<field_type_info>;

template <typename metadata, typename... fields>
struct struct_field_list_impl : struct_field_list_base, fields... {
  using list_metadata = metadata;

  struct_field_list_impl() = default;

  template <typename field_accessor>
    // todo custom optional to use as nttp for cleaner template
    requires (lookup_field<list_metadata>(as_sv(field_accessor::field_id)) != std::nullopt)
  constexpr auto& operator[](field_accessor)  {
    constexpr auto res = lookup_field<list_metadata>(as_sv(field_accessor::field_id));
    using field_type_ref = meta::type_of<res->id>&;
    return static_cast<field_type_ref>(*this).value;
  }

  template <typename field_accessor>
    requires (lookup_field<list_metadata>(as_sv(field_accessor::field_id)) != std::nullopt)
  constexpr const auto& operator[](field_accessor) const {
    constexpr auto res = lookup_field<list_metadata>(as_sv(field_accessor::field_id));
    using field_type_cref = const meta::type_of<res->id>&;
    return static_cast<field_type_cref>(*this).value;
  }
};
} /* namespace s2s */


#endif // _FIELD_LIST_HPP_
