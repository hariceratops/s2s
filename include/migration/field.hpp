#ifndef _FIELD__HPP_
#define _FIELD__HPP_


#include "../field_constraints.hpp"
#include "../fixed_string.hpp"
#include "../field_base.hpp"

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          auto present_only_if,
          auto type_deducer> 
struct field: public field_base<id, T> {
  static constexpr auto constraint_checker = constraint_on_value;
  static constexpr auto field_presence_checker = present_only_if;
  static constexpr auto type_deduction_guide = type_deducer;
};

#endif // _FIELD__HPP_
