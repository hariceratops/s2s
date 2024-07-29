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
struct field: public field_base<id, T> {};

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
*       match_case<.., t1>,
*       match_case<.., t2>
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
*       >, type>,
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
