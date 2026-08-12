#ifndef _READ_IMPL_HPP_
#define _READ_IMPL_HPP_


#include <expected>
#include <bit>
#include <array>
#include <type_traits>

#include "../error/cast_error.hpp"
#include "../lib/s2s_traits/type_traits.hpp"
#include "../lib/memory/address_manip.hpp"
#include "../stream/byte_order.hpp"


namespace s2s {
template <typename T, identified_as_constexpr_stream stream>
constexpr auto read_native_impl(stream& s, T& obj, std::size_t size_to_read) -> rw_result {
  auto as_byte_buffer_rep = as_byte_buffer<stream>(obj);
  if(!s.read(as_byte_buffer_rep, size_to_read)) {
    return std::unexpected(error_reason::buffer_exhaustion);
  }
  if constexpr(std::is_array_v<T>) {
    // bit_cast cannot yield a C array — it returns by value. The buffer goes
    // to the equivalent std::array instead and the elements are assigned.
    using element = std::remove_extent_t<T>;
    auto as_std_array = std::bit_cast<std::array<element, std::extent_v<T>>>(as_byte_buffer_rep);
    for(std::size_t idx = 0; idx < std::extent_v<T>; ++idx)
      obj[idx] = as_std_array[idx];
  } else {
    obj = std::bit_cast<T>(as_byte_buffer_rep);
  }
  return {};
}

template <typename T, readable stream>
constexpr auto read_native_impl(stream& s, T& obj, std::size_t size_to_read) -> rw_result {
  if(!s.read(byte_addressof<stream>(obj), size_to_read)) {
    return std::unexpected(error_reason::buffer_exhaustion);
  }
  return {};
}

template <constant_sized_like T, input_stream_like stream>
constexpr auto read_native(stream& s, T& obj, std::size_t size_to_read) -> rw_result {
  return read_native_impl(s, obj, size_to_read);   
}

template <variable_sized_buffer_like T, input_stream_like stream>
constexpr auto read_native(stream& s, T& obj, std::size_t len_to_read) -> rw_result {
  obj.resize(len_to_read);
  if constexpr(identified_as_constexpr_stream<stream>) {
    // Mirrors write_native: a vector cannot be bit_cast during constant
    // evaluation, so fill it element by element.
    for(auto& elem: obj) {
      auto res = read_native_impl(s, elem, sizeof(elem));
      if(!res)
        return res;
    }
    return {};
  } else {
    return read_native_impl(s, obj, len_to_read * sizeof(T{}[0]));
  }
}

template <trivial T, input_stream_like stream>
constexpr auto read_foreign_scalar(stream& s, T& obj, std::size_t size_to_read) -> rw_result {
  auto res = read_native_impl(s, obj, size_to_read);
  if(res) {
    // todo rollout byteswap if freestanding compiler doesnt provide one
    obj = std::byteswap(obj);
    return {};
  }
  return res;
}

template <buffer_like T, input_stream_like stream>
constexpr auto read_foreign_buffer(stream& s, T& obj, std::size_t len_to_read) -> rw_result {
  auto res = read_native(s, obj, len_to_read);
  if(res) {
    byteswap_elements(obj);
    return {};
  }
  return res;
}

template <std::endian endianness, typename T, input_stream_like stream>
constexpr auto read_impl(stream& s, T& obj, std::size_t N) -> rw_result {
  auto constexpr byte_order = deduce_byte_order<endianness>();
  if constexpr(byte_order == cast_endianness::host) {
    return read_native(s, obj, N); 
  } else if constexpr(byte_order == cast_endianness::foreign) {
    if constexpr(trivial<T>) {
      return read_foreign_scalar(s, obj, N);
    } else if constexpr(buffer_like<T>) {
      return read_foreign_buffer(s, obj, N);
    }
  }
}


} /* namespace s2s */

#endif /* _READ_IMPL_HPP_ */
