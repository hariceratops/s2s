#ifndef _BYTE_ORDER_HPP_
#define _BYTE_ORDER_HPP_


#include <bit>
#include <cstdint>

#include "../lib/s2s_traits/type_traits.hpp"


namespace s2s {
// std::byteswap admits only integral types, and `trivial` admits floats too.
// A float/double is swapped through the unsigned integer of the same width —
// bit_cast there, byteswap, bit_cast back — since there is no portable way to
// flip the bytes of a floating-point object directly. `long double`'s
// extended representation has no such width and no portable wire form, so it
// is rejected at compile time rather than silently mis-swapped.
template <std::size_t N>
struct uint_of_size;
template <> struct uint_of_size<2> { using type = std::uint16_t; };
template <> struct uint_of_size<4> { using type = std::uint32_t; };
template <> struct uint_of_size<8> { using type = std::uint64_t; };
template <std::size_t N>
using uint_of_size_t = typename uint_of_size<N>::type;

template <trivial T>
constexpr auto byteswapped(T v) -> T {
  if constexpr(std::is_integral_v<T>) {
    return std::byteswap(v);
  } else {
    static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8,
                  "a floating-point field is byte-swapped through an unsigned "
                  "integer of the same width; this type has no such width and "
                  "no portable wire form");
    using rep = uint_of_size_t<sizeof(T)>;
    return std::bit_cast<T>(std::byteswap(std::bit_cast<rep>(v)));
  }
}

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
        elem = byteswapped(elem);
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

// The runtime counterpart: a resolved order is a runtime value by
// construction (a byte-order-announcing record decides it from stream
// contents), so the collapse from the schema's std::endian vocabulary to the
// engine's cast_endianness needs a form that does not require a compile-time
// constant.
constexpr auto deduce_byte_order(std::endian endianness) -> cast_endianness {
  return std::endian::native == endianness ? cast_endianness::host : cast_endianness::foreign;
}
} /* namespace s2s */

#endif // _BYTE_ORDER_HPP_
