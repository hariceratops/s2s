#ifndef _STRUCT_FIELD_LIST_HPP_
#define _STRUCT_FIELD_LIST_HPP_


#include <array>
#include "../lib/algorithms/algorithms.hpp"
#include "../lib/containers/static_set.hpp"
#include "../lib/s2s_traits/type_traits.hpp"
#include "../field_list/field_list_metadata.hpp"
#include "../field_list/field_list.hpp"


namespace s2s {
template <typename... fields>
concept all_field_like = (field_like<fields> && ...);

template <std::size_t N>
constexpr bool are_field_ids_unique(const std::array<std::string_view, N>& field_id_list) {
  static_set<std::string_view, N> field_id_set(field_id_list);
  return equal_ranges(field_id_list, field_id_set);
}

template <std::size_t N>
constexpr auto as_sv(const fixed_string<N>& str) {
  return std::string_view{str.data()};
}

template <typename... fields>
concept has_unique_field_ids = are_field_ids_unique(std::array{as_sv(fields::field_id)...});

template <typename metadata>
struct dependency_check {
  static constexpr bool size_ok = size_dependencies_resolved<metadata>();
  static constexpr bool parse_ok = parse_dependencies_resolved<metadata>();
  static constexpr bool type_ok = type_deduction_dependencies_resolved<metadata>();

  static_assert(size_ok, "Size dependencies not resolved");
  static_assert(parse_ok, "Parse dependencies not resolved");
  static_assert(type_ok, "Type deduction dependencies not resolved");

  static constexpr bool all_ok = size_ok && parse_ok && type_ok;
};

template <typename metadata>
concept all_dependencies_resolved = dependency_check<metadata>::all_ok;

template <typename... fields>
  requires (all_dependencies_resolved<field_list_metadata<fields...>>)
struct create_struct_field_list {
  using metadata = field_list_metadata<fields...>;
  using value = struct_field_list_impl<metadata, fields...>;
};

template <typename... fields>
  requires all_field_like<fields...> &&
           has_unique_field_ids<fields...>
using struct_field_list = create_struct_field_list<fields...>::value;
}

#endif /* _STRUCT_FIELD_LIST_HPP_ */

