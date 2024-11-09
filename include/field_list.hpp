#ifndef _FIELD_LIST__HPP_
#define _FIELD_LIST__HPP_

#include "field_size.hpp"
#include "field_traits.hpp"
#include "field_lookup.hpp"


template <typename... ts>
struct are_all_fields;

template <>
struct are_all_fields<field_list<>> { static constexpr bool all_same = true; };

template <typename T, typename... rest>
struct are_all_fields<field_list<T, rest...>> {
  static constexpr bool all_same = false;
};

template <field_like T, typename... rest>
struct are_all_fields<field_list<T, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};

template <typename T>
inline constexpr bool are_all_fields_v = are_all_fields<T>::all_same;

// template <are_all_fields... fields>
template <typename... fields>
struct struct_field_list : struct_field_list_base, fields... {
  // static_assert(are_all_fields_v<field_list<fields...>>, 
  //               "struct_field_list shall be templated with field like types only");
  // todo: impl size resolution
  // todo: impl dependencies resolution
  // static_assert(size_indices_resolved_v<field_list<fields...>>, 
  //   "sizes not resolved. check if any of the fields which depends on the value of another field, \
  //    is always to the left of the dependant field and the field it depends on exists ");
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

#endif // _FIELD_LIST__HPP_
