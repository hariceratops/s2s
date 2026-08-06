#ifndef _TYPE_DEDUCTION_METAFUNCTIONS_HPP_
#define _TYPE_DEDUCTION_METAFUNCTIONS_HPP_


#include "../../lib/metaprog/mp.hpp"
#include "../../lib/containers/static_vector.hpp"
#include "../../lib/containers/static_set.hpp"
#include "../../field_compute/computation_from_fields.hpp"
#include "../if_else_ladder/ladder.hpp"
#include "type.hpp"


namespace s2s {
template <typename... type_tags>
struct extract_type_from_tags {
  static constexpr auto type_tag_count = sizeof...(type_tags);
  using type_id_vec = static_vector<meta::type_identifier, type_tag_count>;
  static constexpr auto value = type_id_vec(meta::type_id<typename type_tags::type>...);
};

template <typename T>
struct extract_field_choices;

template <
  fixed_string matched_id, 
  template<typename...> typename type_switch,
  auto... match_values, typename... type_tags
>
struct extract_field_choices<
  type<
    match_field<matched_id>, 
    type_switch<
      match_case<match_values, type_tags>...
    >
  >
>
{
  static constexpr auto value = extract_type_from_tags<type_tags...>::value;
};

template <
  auto callable, typename R, typename field_name_list,
  template<typename...> typename type_switch,
  auto... match_values, typename... type_tags
>
struct extract_field_choices<
  type<
    compute<callable, R, field_name_list>, 
    type_switch<
      match_case<match_values, type_tags>...
    >
  >
>
{
  static constexpr auto value = extract_type_from_tags<type_tags...>::value;
};

template <
  auto... callables, typename... field_name_lists, typename... type_tags
>
struct extract_field_choices<
  type<
    type_if_else<
      branch<compute<callables, bool, field_name_lists>, type_tags>...
    >
  >
>
{
  static constexpr auto value = extract_type_from_tags<type_tags...>::value;
};

template <std::size_t N>
constexpr bool has_unique_field_choices(const s2s::static_vector<meta::type_identifier, N>& type_id_list) {
  static_set<meta::type_identifier, N> type_id_set(type_id_list);
  return equal_ranges(type_id_list, type_id_set);
}


// has_unique_field_choices enforces unique case *types*, which is what makes
// alternative-to-index inversion well-defined. It says nothing about the case
// *values*, and duplicates there break round-trip silently: writing the second
// alternative emits value v, and reading v back selects the first case that
// matches it. Only the switch forms carry values; a ladder has none.
template <typename T>
struct extract_match_values {
  static constexpr auto value = static_vector<std::size_t, 1>();
};

template <
  fixed_string matched_id,
  template<typename...> typename type_switch,
  auto... match_values, typename... type_tags
>
struct extract_match_values<
  type<
    match_field<matched_id>,
    type_switch<
      match_case<match_values, type_tags>...
    >
  >
>
{
  static constexpr auto value =
    static_vector<std::size_t, sizeof...(match_values)>(static_cast<std::size_t>(match_values)...);
};

template <
  auto callable, typename R, typename field_name_list,
  template<typename...> typename type_switch,
  auto... match_values, typename... type_tags
>
struct extract_match_values<
  type<
    compute<callable, R, field_name_list>,
    type_switch<
      match_case<match_values, type_tags>...
    >
  >
>
{
  static constexpr auto value =
    static_vector<std::size_t, sizeof...(match_values)>(static_cast<std::size_t>(match_values)...);
};

template <std::size_t N>
constexpr bool has_unique_match_values(const s2s::static_vector<std::size_t, N>& match_value_list) {
  static_set<std::size_t, N> match_value_set(match_value_list);
  return equal_ranges(match_value_list, match_value_set);
}
}

#endif /* _TYPE_DEDUCTION_METAFUNCTIONS_HPP_ */
