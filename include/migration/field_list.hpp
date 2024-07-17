#ifndef _FIELD_LIST__HPP_
#define _FIELD_LIST__HPP_

#include "compute_res.hpp"
#include "field_.hpp"
#include "field_lookup_.hpp"
#include "sc_type_traits.hpp"


template <typename T>
struct is_field;

template <fixed_string id,
          field_containable T, 
          typename size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_field<field_<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename... ts>
struct are_all_fields;

template <>
struct are_all_fields<field_list<>> { static constexpr bool all_same = true; };

template <fixed_string id, typename T, typename size, auto field_constraint>
struct are_all_fields<field_list<field_<id, T, size, field_constraint>>> {
  static constexpr bool all_same = true;
};

template <fixed_string id, typename T>
struct are_all_fields<field_list<struct_field<id, T>>> {
  static constexpr bool all_same = true;
};

template <typename T, typename... rest>
struct are_all_fields<field_list<T, rest...>> {
  static constexpr bool all_same = false;
};

template <typename T>
struct are_all_fields<T> {
  static constexpr bool all_same = false;
};

template <fixed_string first_id, field_containable T, typename size, auto field_constraint,
          typename... rest>
struct are_all_fields<field_list<field<first_id, T, size, field_constraint>, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};

template <fixed_string first_id, typename T, typename... rest>
struct are_all_fields<field_list<struct_field<first_id, T>, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};

template <fixed_string first_id, typename T, typename size, auto field_constraint,
          typename... rest>
struct are_all_fields<field_list<runtime_field<first_id, T, size, field_constraint>, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};

template <typename T>
inline constexpr bool are_all_fields_v = are_all_fields<T>::all_same;



template <typename... fields>
struct struct_field_list_ : struct_field_list_base, fields... {
  static_assert(are_all_fields_v<field_list<fields...>>, 
                "struct_field_list shall be templated with field like types only");
  static_assert(size_indices_resolved_v<field_list<fields...>>, 
    "sizes not resolved. check if any of the fields which depends on the value of another field, \
     is always to the left of the dependant field and the field it depends on exists ");

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
