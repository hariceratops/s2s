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

/*
* user side types / metafunctions
* maybe_field<
*   fixed_string,
*   type,
*   constraint_on_value,
*   eval_result<expression<callable>, with_fields<...>>
* >
* union_field<
*   fixed_string, 
*   type<
*     eval_result<
*       expression<callable>, 
*       with_fields<...>, 
*     >,
*     type_switch<
*       match_case<.., type_tag_1>,
*       match_case<.., type_tag_2>
*     >
*   >
* >
* union_field<
*   fixed_string,
*   type<
*     type_switch<
*       eval_result<
*         expression<callable>,
*         with_fields<...>,
*       >, type_tag>,
*       ...
*     >
*   >
* >
* union_impl =>
* field<fixed_string, 
*       std::variant<eval_result::type...>,
*       field_size<sizeof(eval_result::type)...>,
*       no_constraint,
*       always_present,
*       ?>
*template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          auto present_only_if,
          auto type_eval> 
struct field: public field_base<id, T> {};

*/


#endif // _FIELD__HPP_
