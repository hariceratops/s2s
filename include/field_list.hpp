#ifndef _FIELD_LIST_HPP_
#define _FIELD_LIST_HPP_


#include "typelist.hpp"
#include "fixed_string.hpp"
#include "field_list_base.hpp"
#include "field_traits.hpp"
#include "field_list_metafunctions.hpp"


namespace s2s {
template <fixed_string... arg>
struct are_unique_fixed_strings;

template <fixed_string head>
struct are_unique_fixed_strings<head> {
  static constexpr bool res = true;
};

template <fixed_string head, fixed_string neck, fixed_string... tail>
struct are_unique_fixed_strings<head, neck, tail...> {
  constexpr static bool res =
    ((head != neck) && ... && (head != tail)) &&
    are_unique_fixed_strings<neck, tail...>::res;
};


template <fixed_string... field_ids>
inline constexpr bool has_unique_field_ids_v = are_unique_fixed_strings<field_ids...>::res;


template <typename... fields>
concept all_field_like = (field_like<fields> && ...);

 
// template <typename flist, typename glist>
// struct size_indices_resolved;
//  
// // if the second list is empty, we have processed all the fields without short circuiting
// // so we can treat that we have checked all the fields and all checks have passed
// template <typename... xs>
// struct size_indices_resolved<
//   field_list<xs...>, 
//   field_list<>> {
//   static constexpr bool is_resolved = true;
// };
//  
// template <fixed_string id, typename T, typename size, auto constraint, typename... rest>
// struct size_indices_resolved<
//   field_list<>, 
//   field_list<field<id, T, size, constraint>, rest...>> {
//   static constexpr bool is_resolved = 
//     true && 
//     size_indices_resolved<
//       field_list<field<id, T, field_size<size>, constraint>>, 
//       field_list<rest...>
//     >::is_resolved;
// };

// // a struct nested inside a struct shall not have a size dependant on another field, so we can skip
// template <fixed_string id, typename T, typename... head, typename... rest>
// struct size_indices_resolved<
//   field_list<head...>, 
//   field_list<struct_field<id, T>, rest...>> {
//   static constexpr bool is_resolved = 
//     true &&
//     size_indices_resolved<
//       field_list<head..., struct_field<id, T>>, 
//       field_list<rest...>
//     >::is_resolved;
// };
 
// // if the first field has the size dependant on a field, then its size cannot be deduced
// template <fixed_string id, typename T, fixed_string size_source, auto constraint, typename... rest>
// struct size_indices_resolved<
//   field_list<>, 
//   field_list<field<id, T, field_size<size_source>, constraint>, rest...>> {
//   field_list<runtime_field<id, T, runtime_size<from_field<size_source>>, constraint>, rest...>> {
//   static constexpr bool is_resolved = false;
// };
//  
// template <fixed_string id, typename T, std::size_t size, auto constraint, 
//           typename x, typename... xs, typename... rest>
// struct size_indices_resolved<
//   field_list<x, xs...>, 
//   field_list<field<id, T, field_size<size>, constraint>, rest...>> {
//   static constexpr bool is_resolved =
//     true &&
//     size_indices_resolved<
//       field_list<x, xs..., field<id, T, field_size<size>, constraint>>, 
//       field_list<rest...>
//     >::is_resolved;
// };
//  
// template <fixed_string id, typename T, fixed_string size_source, auto constraint, 
//           typename x, typename... xs, typename... rest>
// struct size_indices_resolved<
//   field_list<x, xs...>, 
//   field_list<field<id, T, field_size<size_source>, constraint>, rest...>> {
//   field_list<runtime_field<id, T, runtime_size<from_field<size_source>>, constraint>, rest...>> {
//  
//   using f = field_lookup_v<field_list<x, xs...>, size_source>;
//   static constexpr bool is_resolved =
//     !std::is_same_v<f, field_lookup_failed> &&
//     // std::is_same_v<typename f::field_type, std::size_t> &&
//     // todo ensure that the field type is integral
//     // std::is_same_v<extract_type_from_field_v<f>, std::size_t> &&
//     size_indices_resolved<
//       field_list<x, xs..., field<id, T, field_size<size_source>, constraint>>, 
//       field_list<x, xs..., field<id, T, runtime_size<from_field<size_source>>, constraint>>, 
//       field_list<rest...>
//     >::is_resolved;
// };
//  
// template <typename list>
// inline constexpr bool size_indices_resolved_v = size_indices_resolved<typelist::list<>, list>::is_resolved;
//

// todo: impl size resolution size_indices_resolved_v<typelist::list<fields...>>
// todo: impl dependencies resolution
template <typename... fields>
  requires all_field_like<fields...> &&
           has_unique_field_ids_v<fields::field_id...>
struct struct_field_list : struct_field_list_base, fields... {
  struct_field_list() = default;
  template <typename field_accessor, 
            typename field = field_lookup_v<typelist::list<fields...>, field_accessor::field_id>>
    requires (!std::is_same_v<field_lookup_failed, field>)
  constexpr auto& operator[](field_accessor)  {
    return static_cast<field&>(*this).value;
  }

  template <typename field_accessor,
            typename field = field_lookup_v<typelist::list<fields...>, field_accessor::field_id>>
    requires (!std::is_same_v<field_lookup_failed, field>)
  constexpr const auto& operator[](field_accessor) const {
    return static_cast<const field&>(*this).value;
  }
};
} /* namespace s2s */


#endif // _FIELD_LIST_HPP_
