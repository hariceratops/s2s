#ifndef _LADDER_HPP_
#define _LADDER_HPP_


#include "clause_traits.hpp"
#include "../utils/helper.hpp"


namespace s2s {
template <branch_like... branches>
  requires (sizeof...(branches) > 0)
struct type_if_else {
  // todo possibly unused
  using variant = variant_from_type_conditions_v<branches...>;
  using sizes = size_choices_from_type_conditions_v<branches...>;
};
} /* namespace s2s */

#endif // _LADDER_HPP_
