#ifndef _FIELD_LIST_METADATA_HPP_
#define _FIELD_LIST_METADATA_HPP_

#include "mp.hpp"
#include "containers.hpp"
#include "fixed_string.hpp"
#include "field.hpp"
#include "field_size.hpp"
#include "field_type_info.hpp"
#include "type_deduction.hpp"
#include "type_deduction_clause.hpp"

namespace s2s {
// todo fix these numbers and possibly generate them
static inline constexpr std::size_t max_dep_count_per_field = 8;
static inline constexpr std::size_t max_union_choices = 8;
static inline constexpr std::size_t max_dep_count_per_struct = max_dep_count_per_field * max_union_choices;
static inline constexpr std::size_t max_field_count = 256;


using sv = std::string_view;
using dep_vec = static_vector<sv, max_dep_count_per_struct>;
using field_table_t = static_map<sv, field_type_info, max_field_count>;
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
constexpr auto flatten(const dep_vec (&vecs)[N]) -> dep_vec {
  dep_vec vec;
  for(auto i = 0u; i < N; i++) {
    for(auto& elem: vecs[i]) {
      vec.push_back(elem);
    }
  }
  return vec;
}

template <typename... Ts>
struct extract_length_dependencies_from_field_choices;

template <typename... Ts>
struct extract_length_dependencies_from_field_choices<field_choice_list<Ts...>>{
  static constexpr dep_vec deps[64] = {dep_vec(extract_length_dependencies<Ts>::value)...};
  static constexpr auto value = flatten(deps);
};

template <typename... Ts>
inline constexpr auto extract_length_dependencies_from_field_choices_v = extract_length_dependencies_from_field_choices<Ts...>::value;

template <fixed_string id, typename type_deducer, auto choices>
struct extract_length_dependencies<
  union_field<id, type_deducer, choices>
> 
{
  using field = union_field<id, type_deducer, choices>;
  using field_choices = typename field::field_choices;
  static constexpr auto value = extract_length_dependencies_from_field_choices_v<field_choices>;
};
 

template <typename T>
inline constexpr auto extract_length_dependencies_v = extract_length_dependencies<T>::value;


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


template <typename T>
struct extract_type_deduction_dependencies;

template <typename T>
struct extract_type_deduction_dependencies {
  static constexpr auto value = static_vector<sv, max_dep_count_per_struct>();
};

template <fixed_string id, fixed_string matched_id, type_switch_like type_switch, auto field_choices>
struct extract_type_deduction_dependencies<
  union_field<
    id,
    type<match_field<matched_id>, type_switch>,
    field_choices
  >
> 
{
  static constexpr auto value = dep_vec(as_sv(matched_id));
};

template <fixed_string id, auto callable, typename R, fixed_string... req_fields, type_switch_like type_switch, auto field_choices>
struct extract_type_deduction_dependencies<
  union_field<
    id,
    type<compute<callable, R, fixed_string_list<req_fields...>>, type_switch>,
    field_choices
  >
> 
{
  static constexpr auto value = dep_vec(as_sv(req_fields)...);
};

template <typename T>
struct extract_req_fields_from_clause;

template <auto callable, fixed_string... req_fields, type_tag_like T>
struct extract_req_fields_from_clause<
  branch<
    compute<callable, bool, fixed_string_list<req_fields...>>,
    T
  >
>
{
  static constexpr auto value = dep_vec(as_sv(req_fields)...);
};

template <typename T>
inline constexpr auto extract_req_fields_from_clause_v = extract_req_fields_from_clause<T>::value;

// todo better implementation
constexpr auto remove_duplicates(const dep_vec& vec) -> dep_vec {
  static_set<sv, vec.capacity()> set(vec);
  dep_vec res;
  for(auto item: set)
    res.push_back(item);
  return res;
}

// template<typename...>... typename clauses?
template <fixed_string id, auto field_choices, typename... clauses>
struct extract_type_deduction_dependencies<
  union_field<
    id,
    type<type_if_else<clauses...>>,
    field_choices
  >
> 
{
  static constexpr dep_vec deps[64] = {dep_vec(extract_req_fields_from_clause_v<clauses>)...};
  static constexpr auto flat_values = flatten(deps);
  static constexpr auto value = remove_duplicates(flat_values);
};

template <typename T>
inline constexpr auto extract_type_deduction_dependencies_v = extract_type_deduction_dependencies<T>::value;

template <typename... fields>
struct field_list_metadata {
  template <std::size_t... Is>
  static constexpr auto generate_field_table(std::index_sequence<Is...>) {
    return static_map<sv, field_type_info, max_field_count>(
      {
        {as_sv(fields::field_id), field_type_info(meta::type_id<fields>, Is)}...
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

  static constexpr auto generate_type_deduction_dependency_table() {
    return static_map<sv, static_vector<sv, max_dep_count_per_struct>, max_field_count>(
      {
        {as_sv(fields::field_id), extract_type_deduction_dependencies_v<fields>}...
      }
    );
  }

  static constexpr field_table_t field_table = generate_field_table(std::make_index_sequence<sizeof...(fields)>{});
  static constexpr dependency_table_t length_dependency_table = generate_len_dep_table();
  static constexpr dependency_table_t parse_dependency_table = generate_parse_dependency_table();
  static constexpr dependency_table_t type_deduction_dep_table = generate_type_deduction_dependency_table();
 
};

template <typename list_metadata>
constexpr auto lookup_field(sv field_name) -> std::optional<field_type_info> {
  auto field_table = list_metadata::field_table;
  return field_table[field_name];
}


constexpr bool is_dependencies_resolved(const field_table_t& field_table, const dependency_table_t& dependency_table) {
  for(auto& entry: dependency_table) {
    auto& [field_name, dependencies] = *entry; 
    auto field_info = field_table[field_name];
    auto field_idx = field_info->occurs_at_idx;
    if(dependency_table.size() > 0) {
      for(auto dep_field: dependencies) {
        auto dep_field_info = field_table[dep_field];
        auto dep_field_idx = dep_field_info->occurs_at_idx;
        if(dep_field_idx > field_idx)
          return false;
      }
    }
  }
  return true;
}


// todo use algorithms over raw loops
template <typename list_metadata>
constexpr bool size_dependencies_resolved() {
  auto field_table = list_metadata::field_table;
  auto length_dependency_table = list_metadata::length_dependency_table;
  return is_dependencies_resolved(field_table, length_dependency_table);
}

template <typename list_metadata>
constexpr bool parse_dependencies_resolved() {
  auto field_table = list_metadata::field_table;
  auto parse_dependency_table = list_metadata::parse_dependency_table;
  return is_dependencies_resolved(field_table, parse_dependency_table);
}

template <typename list_metadata>
constexpr bool type_deduction_dependencies_resolved() {
  auto field_table = list_metadata::field_table;
  auto type_deduction_dep_table = list_metadata::type_deduction_dep_table;
  return is_dependencies_resolved(field_table, type_deduction_dep_table);
}

}

#endif /* _FIELD_LIST_METADATA_HPP_ */
