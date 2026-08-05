#ifndef _BYTE_ORDER_HPP_
#define _BYTE_ORDER_HPP_


#include <bit>


namespace s2s {
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
