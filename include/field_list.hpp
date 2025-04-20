#ifndef _FIELD_LIST_HPP_
#define _FIELD_LIST_HPP_

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

// todo: impl size resolution size_indices_resolved_v<field_list<fields...>>
// todo: impl dependencies resolution
template <typename... fields>
  requires all_field_like<fields...> &&
           has_unique_field_ids_v<fields::field_id...>
struct struct_field_list : struct_field_list_base, fields... {
  struct_field_list() = default;
  template <typename field_accessor, 
            typename field = field_lookup_v<field_list<fields...>, field_accessor::field_id>>
    requires (!std::is_same_v<field_lookup_failed, field>)
  constexpr auto& operator[](field_accessor)  {
    return static_cast<field&>(*this).value;
  }

  template <typename field_accessor,
            typename field = field_lookup_v<field_list<fields...>, field_accessor::field_id>>
    requires (!std::is_same_v<field_lookup_failed, field>)
  constexpr const auto& operator[](field_accessor) const {
    return static_cast<const field&>(*this).value;
  }
};
} /* namespace s2s */


#endif // _FIELD_LIST_HPP_
