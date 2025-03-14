#ifndef _TYPE_DEDUCTION_HELPER_HPP_
#define _TYPE_DEDUCTION_HELPER_HPP_


#include "match_case.hpp"
#include "clause.hpp"


template <typename T>
concept type_condition_like = match_case_like<T> || clause_like<T>;

template <type_condition_like match_case>
struct type_from_type_condition;

template <type_condition_like match_case>
struct type_from_type_condition {
  using type = typename match_case::type_tag::type;
};

template <type_condition_like match_case>
struct size_from_type_condition;

template <type_condition_like match_case>
struct size_from_type_condition {
  using size = typename match_case::type_tag::size;
};

template <typename T>
using type_from_type_condition_v = type_from_type_condition<T>::type;

template <typename T>
using size_from_type_condition_v = size_from_type_condition<T>::size;

template <type_condition_like... cases>
struct variant_from_type_conditions {
  using variant = std::variant<type_from_type_condition_v<cases>...>;
};

template <type_condition_like... cases>
using variant_from_type_conditions_v = variant_from_type_conditions<cases...>::variant;

template <type_condition_like... cases>
struct size_choices_from_type_conditions {
  using choices = field_size<size_choices<size_from_type_condition_v<cases>...>>;
};

template <type_condition_like... cases>
using size_choices_from_type_conditions_v = size_choices_from_type_conditions<cases...>::choices;

#endif // _TYPE_DEDUCTION_HELPER_HPP_
