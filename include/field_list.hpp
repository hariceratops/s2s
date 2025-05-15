#ifndef _FIELD_LIST_HPP_
#define _FIELD_LIST_HPP_


#include <ranges>
#include "field.hpp"
#include "typelist.hpp"
#include "fixed_string.hpp"
#include "field_list_base.hpp"
#include "field_traits.hpp"
#include "field_list_metafunctions.hpp"

#include "algorithms.hpp"
#include "containers.hpp"
#include "mp.hpp"
#include "s2s_type_traits.hpp"


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

// inline constexpr auto a = meta::type_id<std::array<int, 10>>;
// static_assert(10 == meta::invoke<extract_size_from_array>(a));

template <typename... fields>
concept all_field_like = (field_like<fields> && ...);


template <typename field_list, typename field_id_list>
struct bulk_look_up;
 
template <typename flist, typename glist>
struct size_dependencies_resolved;
 
// if the second list is empty, we have processed all the fields without short circuiting
// so we can treat that we have checked all the fields and all checks have passed
template <typename... xs>
struct size_dependencies_resolved<
  typelist::list<xs...>, 
  typelist::list<>
> 
{
  static constexpr bool is_resolved = true;
};


// if the first field has the size dependant on a field, then its size cannot be deduced
template <fixed_string id, typename T, variable_size_like size, auto constraint, typename... rest>
struct size_dependencies_resolved<
  typelist::list<>, 
  typelist::list<field<id, T, size, constraint>, rest...>
> 
{
  static constexpr bool is_resolved = false;
};

template <fixed_string id, typename T, fixed_size_like size, auto constraint, 
          typename... xs, typename... rest>
struct size_dependencies_resolved<
  typelist::list<xs...>, 
  typelist::list<field<id, T, size, constraint>, rest...>
> 
{
  static constexpr bool is_resolved =
    true &&
    size_dependencies_resolved<
      typelist::list<xs..., field<id, T, size, constraint>>, 
      typelist::list<rest...>
    >::is_resolved;
};

template <fixed_string id, typename T, size_dont_care_like size, auto constraint, 
          typename... xs, typename... rest>
struct size_dependencies_resolved<
  typelist::list<xs...>, 
  typelist::list<field<id, T, size, constraint>, rest...>
> 
{
  static constexpr bool is_resolved =
    true &&
    size_dependencies_resolved<
      typelist::list<xs..., field<id, T, size, constraint>>, 
      typelist::list<rest...>
    >::is_resolved;
};

template <fixed_string id, typename T, fixed_string len_source, auto constraint, 
          typename x, typename... xs, typename... rest>
struct size_dependencies_resolved<
  typelist::list<x, xs...>, 
  typelist::list<field<id, T, field_size<len_from_field<len_source>>, constraint>, rest...>
> 
{
  using f = field_lookup_v<typelist::list<x, xs...>, len_source>;
  static constexpr bool is_resolved =
    !std::is_same_v<f, field_lookup_failed> &&
    // todo ensure that the field type is integral
    // std::is_same_v<extract_type_from_field_v<f>, std::size_t> &&
    size_dependencies_resolved<
      typelist::list<x, xs..., field<id, T, field_size<len_from_field<len_source>>, constraint>>, 
      typelist::list<rest...>
    >::is_resolved;
};

template <typename... fields, fixed_string... field_names>
struct bulk_look_up<typelist::list<fields...>, fixed_string_list<field_names...>> {
  using field_list = typelist::list<fields...>;
  static constexpr bool res = !typelist::any_of_v<typelist::list<field_lookup_v<field_list, field_names>...>, field_lookup_failed>;
};

template <fixed_string id, typename T, auto callable, field_name_list req_fields, auto constraint, 
          typename x, typename... xs, typename... rest>
struct size_dependencies_resolved<
  typelist::list<x, xs...>, 
  typelist::list<field<id, T, field_size<len_from_fields<callable, req_fields>>, constraint>, rest...>
> 
{
  static constexpr bool is_resolved =
    bulk_look_up<typelist::list<x, xs...>, req_fields>::res &&
    size_dependencies_resolved<
      typelist::list<x, xs..., field<id, T, field_size<len_from_fields<callable, req_fields>>, constraint>>, 
      typelist::list<rest...>
    >::is_resolved;
};

template <fixed_string id, typename T, typename size, auto constraint, 
          typename present_only_if, typename optional,
          typename... xs, typename... rest>
struct size_dependencies_resolved<
  typelist::list<xs...>, 
  typelist::list<maybe_field<field<id, T, size, constraint>, present_only_if, optional>, rest...>
> 
{
  // we create a dummy field and wrap it in a list to reuse size_indices_resolved
  static constexpr bool is_resolved =
    size_dependencies_resolved<typelist::list<xs...>, typelist::list<field<id, T, size, constraint>>>::is_resolved &&
    size_dependencies_resolved<
      typelist::list<xs..., maybe_field<field<id, T, size, constraint>, present_only_if, optional>>, 
      typelist::list<rest...>
    >::is_resolved;
};

template <typename... fields, typename... field_needles>
struct bulk_look_up<typelist::list<fields...>, field_choice_list<field_needles...>> {
  using haystack = typelist::list<fields...>;
  static constexpr bool res = (... && (size_dependencies_resolved<haystack, typelist::list<field_needles>>::is_resolved));
};

template <fixed_string id, typename type_deducer, typename type,
          auto constraint_on_value, typename variant, typename field_choices_t,
          typename size, typename... xs, typename... rest>
struct size_dependencies_resolved<
  typelist::list<xs...>, 
  typelist::list<union_field<id, type_deducer, type, size, constraint_on_value, variant, field_choices_t>, rest...>
> 
{
  static constexpr bool is_resolved =
    bulk_look_up<typelist::list<xs...>, field_choices_t>::res &&
    // true &&
    size_dependencies_resolved<
      typelist::list<xs..., union_field<id, type_deducer, type, size, constraint_on_value, variant, field_choices_t>>, 
      typelist::list<rest...>
    >::is_resolved;
};

template <typename list>
inline constexpr bool size_dependencies_resolved_v = size_dependencies_resolved<typelist::list<>, list>::is_resolved;


template <std::size_t N>
constexpr bool are_field_ids_unique(const std::array<std::string_view, N>& field_id_list) {
  static_set<std::string_view, N> field_id_set(field_id_list);
  return equal_ranges(field_id_list, field_id_set);
}

// static_assert(check_for_field_id_uniqueness(std::array{std::string_view{"hello"}}));
// static_assert(!check_for_field_id_uniqueness(std::array{std::string_view{"hello"}, std::string_view{"world"}, std::string_view{"hello"}}));

template <std::size_t N>
constexpr auto as_sv(const fixed_string<N>& str) {
  return std::string_view{str.data()};
}

template <typename... fields>
concept has_unique_field_ids = are_field_ids_unique(std::array{as_sv(fields::field_id)...});

template <typename T>
struct extract_field_id;

template <typename metadata, typename... fields>
  // requires all_field_like<fields...> &&
  //          has_unique_field_ids<fields...> &&
  //          size_dependencies_resolved_v<typelist::list<fields...>>
struct struct_field_list_impl : struct_field_list_base, fields... {
  // use a map of name to field type_id, use it for lookup, compile time [] in map if null opt
  // roll out optional type if result not null opt, use type_of
  // a constexpr field_list metadata? with type map, size dependencies and parse dependencies
  using list_metadata = metadata;

  struct_field_list_impl() = default;
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
