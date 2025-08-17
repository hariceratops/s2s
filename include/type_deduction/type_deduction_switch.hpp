#ifndef _TYPE_DEDUCTION_SWITCH_HPP_
#define _TYPE_DEDUCTION_SWITCH_HPP_

#include <expected>
#include "../error/cast_error.hpp"
#include "type_deduction_match_case.hpp"
#include "type_deduction_match_case_traits.hpp"
#include "type_deduction_helper.hpp"


namespace s2s {
template <match_case_like... cases>
  requires (sizeof...(cases) > 0)
struct type_switch {
  using variant = variant_from_type_conditions_v<cases...>;
  using sizes = size_choices_from_type_conditions_v<cases...>;
};
} /* namespace s2s */


#endif // _TYPE_SWITCH_HPP_
