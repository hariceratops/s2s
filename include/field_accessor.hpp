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

// template <std::size_t N>
// constexpr auto from_field(const char (&str)[N]) {
//   return field_accessor<fixed_string(str)>{};
// }

#endif // _FIELD_ACCESSOR_HPP_
