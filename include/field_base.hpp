#ifndef _FIELD_BASE_HPP_
#define _FIELD_BASE_HPP_

#include "fixed_string.hpp"
#include "field_size.hpp"

// todo: constrain size_type
// template <fixed_string id, typename T, field_size_like size_type>
template <fixed_string id, typename T, typename size_type>
struct field_base {
  static constexpr auto field_id = id;
  static constexpr auto field_size = size_type::size;
  T value;
};

#endif // _FIELD_BASE_HPP_
