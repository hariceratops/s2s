#ifndef _COMPTIME_FIELD_SIZE_DEDUCE_HPP_
#define _COMPTIME_FIELD_SIZE_DEDUCE_HPP_


#include "../field_size/field_size.hpp"


namespace s2s {
template <typename T>
struct deduce_field_size;

template <std::size_t N>
struct deduce_field_size<field_size<fixed<N>>> {
  using field_size_type = field_size<fixed<N>>;

  constexpr auto operator()() -> std::size_t {
    return field_size_type::size_type_t::count;
  }
};
} /* namespace s2s */


#endif // _COMPTIME_FIELD_SIZE_DEDUCE_HPP_
