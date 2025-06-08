#ifndef _READ_IMPL_HPP_
#define _READ_IMPL_HPP_


#include <concepts>
#include <expected>
#include <bit>

#include "../error/cast_error.hpp"
#include "../lib/s2s_traits/type_traits.hpp"
#include "../lib/memory/bit.hpp"
#include "../lib/memory/address_manip.hpp"


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

template <typename T, identified_as_constexpr_stream stream>
constexpr auto read_native_impl(stream& s, T& obj, std::size_t size_to_read) -> rw_result {
  auto as_byte_buffer_rep = as_byte_buffer<stream>(obj);
  if(!s.read(as_byte_buffer_rep, size_to_read)) {
    return std::unexpected(error_reason::buffer_exhaustion);
  }
  obj = std::bit_cast<T>(as_byte_buffer_rep);
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
  return read_native_impl(s, obj, len_to_read * sizeof(T{}[0]));
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
    for(auto& elem: obj) 
      obj = std::byteswap(obj);
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


template <output_stream_like stream>
class output_stream {
private:
  stream& s;

public:
  // delete copy constructor?
  template <typename T>
  constexpr auto write(const char* src_mem, std::size_t size_to_read) -> rw_result {
    // eof = buffer_exhaustion
    // bad | fail = io_error
    if(!s.write(src_mem, size_to_read))
      return std::unexpected(error_reason::buffer_exhaustion);
    return {};
  }
};
} /* namespace s2s */

#endif /* _READ_IMPL_HPP_ */
