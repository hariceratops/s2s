#ifndef _FIELD__HPP_
#define _FIELD__HPP_


#include "field_size.hpp"
#include "field_constraints.hpp"
#include "fixed_string.hpp"

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value>
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  field_type value;
};


template <fixed_string id, typename T, typename size_type, auto constraint_on_value>
using to_optional_field = field<id, std::optional<T>, size_type, constraint_on_value>;

template <fixed_string id, 
          typename T, 
          typename size_type, 
          auto constraint_on_value, 
          typename present_only_if,
          typename optional = to_optional_field<id, T, size_type, constraint_on_value>,
          typename base_field = field<id, T, size_type, constraint_on_value>>
class maybe_field : public optional
{
public:
  using field_base_type = base_field;
  using field_presence_checker = present_only_if;
};


template <typename... choices>
struct field_choice_list {};

template <fixed_string id, typename... args>
struct to_field_choices;

template <fixed_string id, typename T, typename field_size>
struct to_field_choice {
  using field = field<id, T, field_size, no_constraint<T>{}>;
};

template <fixed_string id, typename T, typename field_size>
using to_field_choice_v = to_field_choice<id, T, field_size>::field;

template <fixed_string id, typename... types, typename... sizes>
struct to_field_choices<id, std::variant<types...>, field_size<size_choices<sizes...>>> {
  using choices = field_choice_list<to_field_choice_v<id, types, sizes>...>;
};


// todo how to handle constraint_on_value in general
template <fixed_string id,
          typename type_deducer,
          typename type = typename type_deducer::variant,
          typename size_type = typename type_deducer::sizes,
          auto constraint_on_value = no_constraint<type>{},
          typename variant = field<id, type, size_type, constraint_on_value>,
          typename field_choices_t = to_field_choices<id, type, size_type>::choices
  >
struct union_field: public variant {
  using type_deduction_guide = type_deducer;
  using field_choices = field_choices_t;
  static constexpr auto variant_size = std::variant_size_v<type>;
};


#endif // _FIELD__HPP_
