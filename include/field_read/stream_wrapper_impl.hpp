#ifndef _STREAM_WRAPPER_IMPL_HPP_
#define _STREAM_WRAPPER_IMPL_HPP_


#include <concepts>
#include <expected>
#include <bit>

#include "../error/cast_error.hpp"
#include "../lib/s2s_traits/type_traits.hpp"
#include "../stream/stream_traits.hpp"


namespace s2s {
enum cast_endianness {
  host = 0,
  foreign = 1
};

// todo check if this handrolled implementation is required
// template <std::integral T>
// constexpr auto byteswap(T value) -> T {
//   constexpr auto object_size = sizeof(T);
//   auto value_rep = std::bit_cast<std::array<std::byte, object_size>>(value);
//   for(std::size_t fwd_idx = 0, rev_idx = object_size - 1; 
//       fwd_idx <= rev_idx; 
//       ++fwd_idx, --rev_idx) 
//   {
//     auto tmp = value_rep[fwd_idx];
//     value_rep[fwd_idx] = value_rep[rev_idx];
//     value_rep[rev_idx] = tmp;
//   }
//   return std::bit_cast<T>(value_rep);
// }
//
//
// constexpr auto is_little() -> bool {
//   constexpr uint32_t bait = 0xdeadbeef;
//   constexpr auto bait_size = sizeof(bait);
//   auto value_rep = std::bit_cast<std::array<std::byte, bait_size>>(bait);
//   return value_rep[0] == std::byte{0xef};
// }
//
// static_assert(byteswap(0xdeadbeef) == 0xefbeadde);
//
//
// enum endian: uint32_t {
//   little = 0xdeadbeef,
//   big = 0xefbeadde,
//   native = is_little() ? little : big
// };


template <input_stream_like stream, typename T>
char* byte_addressof(T& obj) {
  return reinterpret_cast<char*>(&obj);
}

template <input_stream_like stream, typename T, std::size_t N>
char* byte_addressof(std::array<T, N>& obj) {
  return reinterpret_cast<char*>(obj.data());
}

template <input_stream_like stream, std::size_t N>
char* byte_addressof(fixed_string<N>& obj) {
  return reinterpret_cast<char*>(obj.data());
}

template <input_stream_like stream, typename T>
char* byte_addressof(std::vector<T>& obj) {
  return reinterpret_cast<char*>(obj.data());
}

template <input_stream_like stream>
inline char* byte_addressof(std::string& obj) {
  return reinterpret_cast<char*>(&obj[0]);
}

// todo generate this as configurable parameter
constexpr std::size_t constexpr_buffer_size = 2048;

template <identified_as_constexpr_stream stream, typename T, std::size_t size = sizeof(T)>
constexpr auto as_byte_buffer(T& obj) -> std::array<char, size> {
  return std::bit_cast<std::array<char, size>>(obj);
}

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

#endif /* _STREAM_WRAPPER_IMPL_HPP_ */
