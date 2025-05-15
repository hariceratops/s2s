#ifndef _FIELD_LIST_METADATA_HPP_
#define _FIELD_LIST_METADATA_HPP_

#include <cstdint>
#include "mp.hpp"
#include "containers.hpp"
#include "fixed_string.hpp"
#include "field.hpp"
#include "field_size.hpp"


namespace s2s {
// todo fix these numbers and possibly generate them
static inline constexpr std::size_t max_dep_count_per_field = 8;
static inline constexpr std::size_t max_union_choices = 8;
static inline constexpr std::size_t max_dep_count_per_struct = max_dep_count_per_field * max_union_choices;
static inline constexpr std::size_t max_field_count = 256;

// todo better name
struct field_node {
  meta::type_identifier id;
  std::size_t occurs_at_idx;
};

using sv = std::string_view;
using field_table_t = static_map<sv, field_node, max_field_count>;
using dependency_table_t = static_map<sv, static_vector<sv, max_dep_count_per_struct>, max_field_count>;

// extract dependencies metafunction
template <typename T>
struct extract_length_dependencies;

template <fixed_string id, typename T, fixed_size_like size, auto constraint>
struct extract_length_dependencies<
  field<id, T, size, constraint>
>
{
  static constexpr auto value = static_vector<sv, max_dep_count_per_struct>();
};

template <fixed_string id, typename T, size_dont_care_like size, auto constraint>
struct extract_length_dependencies<
  field<id, T, size, constraint>
>
{
  static constexpr auto value = static_vector<sv, max_dep_count_per_struct>();
};

template <fixed_string id, typename T, fixed_string len_source, auto constraint>
struct extract_length_dependencies<
  field<id, T, field_size<len_from_field<len_source>>, constraint>
>
{
  static constexpr auto value = static_vector<sv, max_dep_count_per_struct>(as_sv(len_source));
};

template <fixed_string id, typename T, auto callable, auto constraint, fixed_string... req_fields>
struct extract_length_dependencies<
  field<id, T, field_size<len_from_fields<callable, fixed_string_list<req_fields...>>>, constraint>
>
{
  static constexpr auto value = static_vector<sv, max_dep_count_per_struct>(as_sv(req_fields)...);
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

template <std::size_t N>
constexpr auto flatten(const static_vector<sv, max_dep_count_per_struct> (&vecs)[N]) -> static_vector<sv, max_dep_count_per_struct> {
  static_vector<sv, max_dep_count_per_struct> vec;
  for(auto i = 0u; i < N; i++) {
    for(auto& elem: vecs[i]) {
      vec.push_back(elem);
    }
  }
  return vec;
}

// constexpr static_vector<sv, max_dep_count> vecs[2] = {static_vector<sv, max_dep_count>("hello", "world"), static_vector<sv, max_dep_count>("foo", "bar")};
// constexpr auto flat = flatten(vecs);
// static_assert(flat[0] == "hello");
// static_assert(flat[3] == "bar");

template <fixed_string id, typename type_deducer, typename type, typename size,
          auto constraint_on_value, typename variant, typename... field_choices>
struct extract_length_dependencies<
  union_field<id, type_deducer, type, size, constraint_on_value, variant, field_choice_list<field_choices...>>
> 
{
  using dep_vec = static_vector<sv, max_dep_count_per_struct>;
  static constexpr dep_vec deps[64] = {dep_vec(extract_length_dependencies<field_choices>::value)...};
  static constexpr auto value = flatten(deps);
};
 

template <typename T>
inline constexpr auto extract_length_dependencies_v = extract_length_dependencies<T>::value;

// template <auto callable, fixed_string... req_fields>
// struct extract_req_fields<parse_if<callable, fixed_string_list<req_fields...>>> {
//
// };
template <auto callable, typename R, field_name_list Fs>
struct compute;

template <typename T>
struct extract_parse_dependencies;

template <typename T>
struct extract_parse_dependencies {
  static constexpr auto value = static_vector<sv, max_dep_count_per_struct>();
};

template <fixed_string id, typename T, typename size, auto constraint, 
          auto callable, fixed_string... req_fields, typename optional>
struct extract_parse_dependencies<
  maybe_field<field<id, T, size, constraint>, compute<callable, bool, fixed_string_list<req_fields...>>, optional>
>
{
  static constexpr auto value = static_vector<sv, max_dep_count_per_struct>(as_sv(req_fields)...);
};

template <typename T>
inline constexpr auto extract_parse_dependencies_v = extract_parse_dependencies<T>::value;

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
    return static_map<sv, static_vector<sv, max_dep_count_per_struct>, max_field_count>(
      {
        {as_sv(fields::field_id), extract_length_dependencies_v<fields>}...
      }
    );
  }

  static constexpr auto generate_parse_dependency_table() {
    return static_map<sv, static_vector<sv, max_dep_count_per_struct>, max_field_count>(
      {
        {as_sv(fields::field_id), extract_parse_dependencies_v<fields>}...
      }
    );
  }

  static constexpr field_table_t field_table = generate_field_table(std::make_index_sequence<sizeof...(fields)>{});
  static constexpr dependency_table_t length_dependency_table = generate_len_dep_table();
  static constexpr dependency_table_t parse_dependency_table = generate_parse_dependency_table();
  static constexpr dependency_table_t type_deduction_dep_table{};
 
};
}

#endif /* _FIELD_LIST_METADATA_HPP_ */
