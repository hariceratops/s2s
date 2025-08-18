#ifndef _SWITCH_HPP_
#define _SWITCH_HPP_

#include <expected>
#include "../../error/cast_error.hpp"
#include "match_case.hpp"
#include "match_case_traits.hpp"
#include "../utils/helper.hpp"


namespace s2s {
template <match_case_like... cases>
  requires (sizeof...(cases) > 0)
struct type_switch {
  using variant = variant_from_type_conditions_v<cases...>;
  using sizes = size_choices_from_type_conditions_v<cases...>;
};
} /* namespace s2s */


#endif // _SWITCH_HPP_
