#ifndef _FIELD_BASE_HPP_
#define _FIELD_BASE_HPP_

#include "fixed_string.hpp"

template <fixed_string id, typename T, std::size_t size>
struct field_base {
  static constexpr auto field_id = id;
  static constexpr std::size_t field_size = size;
  T value;
};

#endif // _FIELD_BASE_HPP_
