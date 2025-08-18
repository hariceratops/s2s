#ifndef _LADDER_HPP_
#define _LADDER_HPP_


#include <expected>
#include "../../error/cast_error.hpp"
#include "clause.hpp"
#include "clause_traits.hpp"
#include "../utils/helper.hpp"


namespace s2s {
template <branch_like... branches>
  requires (sizeof...(branches) > 0)
struct type_if_else {
  using variant = variant_from_type_conditions_v<branches...>;
  using sizes = size_choices_from_type_conditions_v<branches...>;
};
} /* namespace s2s */

#endif // _LADDER_HPP_
