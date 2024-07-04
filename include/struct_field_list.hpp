#ifndef _STRUCT_FIELD_LIST_HPP_
#define _STRUCT_FIELD_LIST_HPP_


#include "field_size.hpp"
#include "typelist.hpp"
#include "field_accessor.hpp"
#include "struct_field_list_base.hpp"
#include "fixed_string.hpp"
#include "field_base.hpp"
#include "field.hpp"
#include "field_lookup.hpp"
#include "field_meta.hpp"


template <typename... ts>
struct are_all_fields;

template <>
struct are_all_fields<field_list<>> { static constexpr bool all_same = true; };

template <fixed_string id, field_containable T, typename size, auto field_constraint>
struct are_all_fields<field_list<field<id, T, size, field_constraint>>> {
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

template <typename flist, typename glist>
struct size_indices_resolved;

// if the second list is empty, we have processed all the fields without short circuiting
// so we can treat that we have checked all the fields and all checks have passed
template <typename... xs>
struct size_indices_resolved<
  field_list<xs...>, 
  field_list<>> {
  static constexpr bool is_resolved = true;
};

template <fixed_string id, typename T, std::size_t size, auto constraint, typename... rest>
struct size_indices_resolved<
  field_list<>, 
  field_list<field<id, T, field_size<size>, constraint>, rest...>> {
  static constexpr bool is_resolved = 
    true && 
    size_indices_resolved<
      field_list<field<id, T, field_size<size>, constraint>>, 
      field_list<rest...>
    >::is_resolved;
};

// a struct nested inside a struct shall not have a size dependant on another field, so we can skip
template <fixed_string id, typename T, typename... head, typename... rest>
struct size_indices_resolved<
  field_list<head...>, 
  field_list<struct_field<id, T>, rest...>> {
  static constexpr bool is_resolved = 
    true &&
    size_indices_resolved<
      field_list<head..., struct_field<id, T>>, 
      field_list<rest...>
    >::is_resolved;
};

// if the first field has the size dependant on a field, then its size cannot be deduced
template <fixed_string id, typename T, fixed_string size_source, auto constraint, typename... rest>
struct size_indices_resolved<
  field_list<>, 
  field_list<runtime_field<id, T, runtime_size<from_field<size_source>>, constraint>, rest...>> {
  static constexpr bool is_resolved = false;
};

template <fixed_string id, typename T, std::size_t size, auto constraint, 
          typename x, typename... xs, typename... rest>
struct size_indices_resolved<
  field_list<x, xs...>, 
  field_list<field<id, T, field_size<size>, constraint>, rest...>> {
  static constexpr bool is_resolved =
    true &&
    size_indices_resolved<
      field_list<x, xs..., field<id, T, field_size<size>, constraint>>, 
      field_list<rest...>
    >::is_resolved;
};

template <fixed_string id, typename T, fixed_string size_source, auto constraint, 
          typename x, typename... xs, typename... rest>
struct size_indices_resolved<
  field_list<x, xs...>, 
  field_list<runtime_field<id, T, runtime_size<from_field<size_source>>, constraint>, rest...>> {
  using f = field_lookup_v<field_list<x, xs...>, size_source>;
  static constexpr bool is_resolved =
    !std::is_same_v<f, field_lookup_failed> &&
    // todo ensure that the field type is integral
    // std::is_same_v<extract_type_from_field_v<f>, std::size_t> &&
    size_indices_resolved<
      field_list<x, xs..., field<id, T, runtime_size<from_field<size_source>>, constraint>>, 
      field_list<rest...>
    >::is_resolved;
};

template <typename list>
inline constexpr bool size_indices_resolved_v = size_indices_resolved<field_list<>, list>::is_resolved;


template <typename... fields>
struct struct_field_list : struct_field_list_base, fields... {
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

namespace static_test {
static_assert(are_all_fields_v<field_list<field<"a", int, field_size<4u>>>>);
static_assert(!are_all_fields_v<field_list<field<"a", int, field_size<4u>>, int>>);
static_assert(!are_all_fields_v<field_list<int, int>>);
static_assert(are_all_fields_v<field_list<>>);
static_assert(are_all_fields_v<field_list<field<"a", int, field_size<4u>>, field<"b", int, field_size<4u>>>>);
static_assert(are_all_fields_v<field_list<field<"a", int, field_size<4u>>, field<"b", fixed_string<4>, field_size<4u>>>>);
static_assert(!are_all_fields_v<field_list<field<"a", int, field_size<4u>>, field<"b", fixed_string<4>, field_size<4u>>, float>>);
static_assert(size_indices_resolved_v<field_list<field<"a", int, field_size<4ul>>, field<"b", fixed_string<4>, field_size<4ul>>>>);
static_assert(!size_indices_resolved_v<field_list<runtime_field<"a", int, runtime_size<from_field<"b">>>, field<"b", fixed_string<4>, field_size<4ul>>>>);
static_assert(size_indices_resolved_v<field_list<field<"a", int, field_size<4ul>>, runtime_field<"b", fixed_string<4>, runtime_size<from_field<"a">>>>>);
static_assert(!size_indices_resolved_v<field_list<field<"a", int, field_size<4ul>>, field<"b", fixed_string<4>, field_size<4ul>>, runtime_field<"c", int, runtime_size<from_field<"d">>>>>);
}

#endif // _STRUCT_FIELD_LIST_HPP_
