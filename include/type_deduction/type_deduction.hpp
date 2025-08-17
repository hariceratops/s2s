#ifndef _TYPE_DEDUCTION_HPP_
#define _TYPE_DEDUCTION_HPP_


#include "../field/field_accessor.hpp"


namespace s2s {
template <typename... Args>
struct type;


template <fixed_string id>
using match_field = field_accessor<id>;

// todo constraints compute like
template <typename eval_expression, typename _switch>
struct type<eval_expression, _switch> {
  using expression = eval_expression;
  using type_switch = _switch;
  using variant = _switch::variant;
  using sizes = _switch::sizes;
};

template <fixed_string id, typename _switch>
struct type<match_field<id>, _switch> {
  using type_switch = _switch;
  using variant = _switch::variant;
  using sizes = _switch::sizes;
};

// todo constraints
template <typename ladder>
struct type<ladder> {
  using type_ladder = ladder;
  using variant = ladder::variant;
  using sizes = ladder::sizes;
};
} /* namespace s2s */


#endif // _TYPE_DEDUCTION_HPP_
