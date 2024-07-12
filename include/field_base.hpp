#ifndef _FIELD_BASE_HPP_
#define _FIELD_BASE_HPP_

#include "fixed_string.hpp"
#include "field_size.hpp"

template <fixed_string id, typename T>
struct field_base {
  static constexpr auto field_id = id;
  T value;
};

#endif // _FIELD_BASE_HPP_
