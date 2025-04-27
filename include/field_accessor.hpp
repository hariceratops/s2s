#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#include "fixed_string.hpp"

namespace s2s {
template <fixed_string id>
struct field_accessor {
  static constexpr auto field_id = id;
};
}

namespace s2s_literals {
template <s2s::fixed_string id>
constexpr auto operator""_f() {
  return s2s::field_accessor<id>{};
}
}

#endif // _FIELD_ACCESSOR_HPP_
