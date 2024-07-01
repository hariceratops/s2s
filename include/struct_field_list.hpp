#ifndef _STRUCT_FIELD_LIST_HPP_
#define _STRUCT_FIELD_LIST_HPP_


#include "typelist.hpp"
#include "struct_field_list_base.hpp"
#include "fixed_string.hpp"
#include "field_base.hpp"
#include "field_lookup.hpp"


template <typename... ts>
struct are_all_fields;

template <>
struct are_all_fields<field_list<>> { static constexpr bool all_same = true; };

template <fixed_string id, field_containable T, std::size_t size>
struct are_all_fields<field_list<field<id, T, size>>> {
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

template <fixed_string first_id, field_containable T, std::size_t size_first,
          typename... rest>
struct are_all_fields<field_list<field<first_id, T, size_first>, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};

template <fixed_string first_id, typename T, typename... rest>
struct are_all_fields<field_list<struct_field<first_id, T>, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};

template <typename T>
inline constexpr bool are_all_fields_v = are_all_fields<T>::all_same;


template <typename... fields>
struct struct_field_list : struct_field_list_base, fields... {
  static_assert(are_all_fields_v<field_list<fields...>>, "struct_field_list shall be templated with field like types only");

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

namespace static_test {
static_assert(are_all_fields_v<field_list<field<"a", int, 4>>>);
static_assert(!are_all_fields_v<field_list<field<"a", int, 4>, int>>);
static_assert(!are_all_fields_v<field_list<int, int>>);
static_assert(are_all_fields_v<field_list<>>);
static_assert(are_all_fields_v<field_list<field<"a", int, 4>, field<"b", int, 4>>>);
static_assert(are_all_fields_v<field_list<field<"a", int, 4>, field<"b", fixed_string<4>, 4>>>);
static_assert(!are_all_fields_v<field_list<field<"a", int, 4>, field<"b", fixed_string<4>, 4>, float>>);
}

#endif // _STRUCT_FIELD_LIST_HPP_
