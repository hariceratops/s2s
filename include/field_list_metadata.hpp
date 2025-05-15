#ifndef _FIELD_LIST_METADATA_HPP_
#define _FIELD_LIST_METADATA_HPP_

#include <cstdint>
#include "mp.hpp"
#include "containers.hpp"
#include "fixed_string.hpp"
#include "field.hpp"
#include "field_size.hpp"


namespace s2s {
static inline constexpr std::size_t max_dep_count = 8;
static inline constexpr std::size_t max_field_count = 256;

// todo better name
struct field_node {
  meta::type_identifier id;
  std::size_t occurs_at_idx;
};

using sv = std::string_view;
using field_table_t = static_map<sv, field_node, max_field_count>;
using dependency_table_t = static_map<sv, static_vector<sv, max_dep_count>, max_field_count>;

// extract dependencies metafunction
template <typename T>
struct extract_length_dependencies;

template <fixed_string id, typename T, fixed_size_like size, auto constraint>
struct extract_length_dependencies<
  field<id, T, size, constraint>
>
{
  static constexpr auto value = static_vector<sv, max_dep_count>();
};

template <fixed_string id, typename T, size_dont_care_like size, auto constraint>
struct extract_length_dependencies<
  field<id, T, size, constraint>
>
{
  static constexpr auto value = static_vector<sv, max_dep_count>();
};

template <fixed_string id, typename T, fixed_string len_source, auto constraint>
struct extract_length_dependencies<
  field<id, T, field_size<len_from_field<len_source>>, constraint>
>
{
  static constexpr auto value = static_vector<sv, max_dep_count>(as_sv(len_source));
};

template <fixed_string id, typename T, auto callable, auto constraint, fixed_string... req_fields>
struct extract_length_dependencies<
  field<id, T, field_size<len_from_fields<callable, fixed_string_list<req_fields...>>>, constraint>
>
{
  static constexpr auto value = static_vector<sv, max_dep_count>(as_sv(req_fields)...);
};

template <fixed_string id, typename T, typename size, auto constraint, 
          typename present_only_if, typename optional>
struct extract_length_dependencies<
  maybe_field<field<id, T, size, constraint>, present_only_if, optional>
>
{
  using f = field<id, T, size, constraint>;
  static constexpr auto value = extract_length_dependencies<f>::value;
};

template <fixed_string id, typename type_deducer, typename type, typename size,
          auto constraint_on_value, typename variant, typename field_choices_t>
struct extract_length_dependencies<
  union_field<id, type_deducer, type, size, constraint_on_value, variant, field_choices_t>
> 
{
  static constexpr auto value = static_vector<sv, max_dep_count>();
};
 

template <typename T>
inline constexpr auto extract_length_dependencies_v = extract_length_dependencies<T>::value;

template <typename... fields>
struct field_list_metadata {
  template <std::size_t... Is>
  static constexpr auto generate_field_table(std::index_sequence<Is...>) {
    return static_map<sv, field_node, max_field_count>(
      {
        {as_sv(fields::field_id), field_node(meta::type_id<fields>, Is)}...
      }
    );
  }

  static constexpr auto generate_len_dep_table() {
    return static_map<sv, static_vector<sv, max_dep_count>, max_field_count>(
      {
        {as_sv(fields::field_id), extract_length_dependencies_v<fields>}...
      }
    );
  }

  static constexpr field_table_t field_table = generate_field_table(std::make_index_sequence<sizeof...(fields)>{});
  static constexpr dependency_table_t length_dependency_table = generate_len_dep_table();
  static constexpr dependency_table_t parse_dependency_table{};
  static constexpr dependency_table_t type_deduction_dep_table{};
 
};
}

#endif /* _FIELD_LIST_METADATA_HPP_ */
