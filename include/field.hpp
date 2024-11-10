#ifndef _FIELD__HPP_
#define _FIELD__HPP_


#include "field_constraints.hpp"
#include "fixed_string.hpp"

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          typename present_only_if,
          typename type_deducer> 
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  using field_presence_checker = present_only_if;
  using type_deduction_guide = type_deducer;
  field_type value;
};

#endif // _FIELD__HPP_
