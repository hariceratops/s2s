#ifndef _TYPE_DEDUCTION_HELPER_HPP_
#define _TYPE_DEDUCTION_HELPER_HPP_


#include <optional>
#include <expected>
#include "../error/cast_error.hpp"
#include "type_deduction_match_case_traits.hpp"
#include "type_deduction_clause_traits.hpp"


namespace s2s {
using type_deduction_idx = std::optional<std::size_t>;
using type_deduction_res = std::expected<std::size_t, error_reason>;

constexpr auto operator|(const type_deduction_idx& res, auto&& callable) -> type_deduction_idx {
  return res ? res : callable();
}

template <typename T>
concept type_condition_like = match_case_like<T> || branch_like<T>;

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
} /* namespace s2s */ 

#endif // _TYPE_DEDUCTION_HELPER_HPP_
