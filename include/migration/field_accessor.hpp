#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#include "fixed_string.hpp"

template <fixed_string id>
struct field_accessor {
  static constexpr auto field_id = id;
};

template <fixed_string id>
constexpr auto operator""_f() {
  return field_accessor<id>{};
}


#endif // _FIELD_ACCESSOR_HPP_
