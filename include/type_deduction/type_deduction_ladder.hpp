#ifndef _TYPE_DEDUCTION_LADDER_HPP_
#define _TYPE_DEDUCTION_LADDER_HPP_

#include "../error/cast_error.hpp"
#include "type_deduction_clause.hpp"
#include "type_deduction_clause_traits.hpp"
#include "type_deduction_helper.hpp"
#include <expected>


namespace s2s {
template <branch_like... branches>
  requires (sizeof...(branches) > 0)
struct type_if_else {
  using variant = variant_from_type_conditions_v<branches...>;
  using sizes = size_choices_from_type_conditions_v<branches...>;
};
} /* namespace s2s */

#endif // _TYPE_LADDER_HPP_
