#ifndef _FIELD_LIST_HPP_
#define _FIELD_LIST_HPP_


#include "field.hpp"
#include "field_type_info.hpp"
#include "typelist.hpp"
#include "fixed_string.hpp"
#include "field_list_base.hpp"
#include "field_traits.hpp"
#include "field_list_metafunctions.hpp"
#include "mp.hpp"


namespace s2s {

template <typename list_metadata>
constexpr auto lookup_field(std::string_view field_name) -> std::optional<field_type_info>;
template <typename list_metadata>
constexpr bool size_dependencies_resolved();
template <typename list_metadata>
constexpr bool parse_dependencies_resolved();
template <typename list_metadata>
constexpr bool type_deduction_dependencies_resolved();

template <typename metadata>
concept all_size_dependencies_resolved = size_dependencies_resolved<metadata>();
template <typename metadata>
concept all_parse_dependencies_resolved = parse_dependencies_resolved<metadata>();
template <typename metadata>
concept all_type_deduction_dependencies_resolved = type_deduction_dependencies_resolved<metadata>();

template <typename metadata, typename... fields>
  requires (
    all_size_dependencies_resolved<metadata> &&
    all_parse_dependencies_resolved<metadata> &&
    all_type_deduction_dependencies_resolved<metadata>
  )
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
