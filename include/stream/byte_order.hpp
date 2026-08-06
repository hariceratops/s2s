#ifndef _BYTE_ORDER_HPP_
#define _BYTE_ORDER_HPP_


#include <bit>

#include "../lib/s2s_traits/type_traits.hpp"


namespace s2s {
// Single-byte elements have no byte order, and fixed_string exposes no
// iterators, so both fall through untouched. Shared by the read and write
// directions, which is why it lives here rather than beside either one.
template <buffer_like T>
constexpr auto byteswap_elements(T& obj) -> void {
  if constexpr(!fixed_string_like<T>) {
    for(auto& elem: obj) {
      // Multi-dimensional aggregates nest, so descend until the scalars.
      if constexpr(buffer_like<std::remove_reference_t<decltype(elem)>>)
        byteswap_elements(elem);
      else if constexpr(sizeof(elem) > 1)
        elem = std::byteswap(elem);
    }
  }
}


enum cast_endianness {
  host = 0,
  foreign = 1
};


template <std::endian endianness>
constexpr cast_endianness deduce_byte_order() {
  if constexpr(std::endian::native == endianness)
    return cast_endianness::host;
  else if constexpr(std::endian::native != endianness)
    return cast_endianness::foreign;
}
} /* namespace s2s */

#endif // _BYTE_ORDER_HPP_
