#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#include "sc_type_traits.hpp"
#include "field_accessor.hpp"


// todo enforce concept constraints for field_size
template <std::size_t N>
struct field_size {
  static constexpr auto size = N;
};

template <fixed_string id>
using from_field = field_accessor<id>;

template <typename field_accessor>
struct runtime_size {
  // static constexpr auto accessor = field_accessor::field_id;
  static constexpr auto accessor = field_accessor{};
};

#endif // _FIELD_SIZE_HPP_
