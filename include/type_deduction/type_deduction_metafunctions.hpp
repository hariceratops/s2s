#ifndef _TYPE_DEDUCTION_METAFUNCTIONS_HPP_
#define _TYPE_DEDUCTION_METAFUNCTIONS_HPP_


#include "../lib/metaprog/mp.hpp"
#include "../lib/containers/static_vector.hpp"
#include "../lib/containers/static_set.hpp"
#include "../field_compute/computation_from_fields.hpp"
#include "type_deduction_switch.hpp"
#include "type_deduction_ladder.hpp"
#include "type_deduction.hpp"


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
}

#endif /* _TYPE_DEDUCTION_METAFUNCTIONS_HPP_ */
