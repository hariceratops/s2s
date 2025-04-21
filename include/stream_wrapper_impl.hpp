#ifndef _STREAM_WRAPPER_IMPL_HPP_
#define _STREAM_WRAPPER_IMPL_HPP_


#include <concepts>
#include <expected>

#include "cast_error.hpp"
#include "s2s_type_traits.hpp"
#include "stream_traits.hpp"


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

template <input_stream_like stream>
class input_stream {
private:
  stream& s;

  template <typename T>
  constexpr auto read_native_impl(T& obj, std::size_t size_to_read) -> rw_result {
    if(!s.read(byte_addressof(obj), size_to_read)) {
      return std::unexpected(error_reason::buffer_exhaustion);
    }
    return {};
  }

  template <constant_sized_like T>
  constexpr auto read_native(T& obj, std::size_t size_to_read) -> rw_result {
    return read_native_impl(obj, size_to_read);   
  }

  template <variable_sized_buffer_like T>
  constexpr auto read_native(T& obj, std::size_t len_to_read) -> rw_result {
    obj.resize(len_to_read);
    return read_native_impl(obj, len_to_read * sizeof(T{}[0]));
  }

  template <trivial T>
  constexpr auto read_foreign_scalar(T& obj, std::size_t size_to_read) -> rw_result {
    auto res = read_native_impl(obj, size_to_read);
    if(res) {
      // todo rollout byteswap if freestanding compiler doesnt provide one
      obj = std::byteswap(obj);
      return {};
    }
    return res;
  }

  template <buffer_like T>
  constexpr auto read_foreign_buffer(T& obj, std::size_t len_to_read) -> rw_result {
    auto res = read_native(obj, len_to_read);
    if(res) {
      for(auto& elem: obj) 
        obj = std::byteswap(obj);
      return {};
    }
    return res;
  }

public:
  input_stream(stream& s): s(s) {}
  input_stream(const input_stream&) = delete;

  template <std::endian endianness, typename T>
  constexpr auto read(T& obj, std::size_t N) -> rw_result {
    auto constexpr byte_order = deduce_byte_order<endianness>();
    if constexpr(byte_order == cast_endianness::host) {
      return read_native(obj, N); 
    } else if constexpr(byte_order == cast_endianness::foreign) {
      if constexpr(trivial<T>) {
        return read_foreign_scalar(obj, N);
      } else if constexpr(buffer_like<T>) {
        return read_foreign_buffer(obj, N);
      }
    }
  }
};


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

template <typename S>
struct is_s2s_input_stream;

template <typename S>
struct is_s2s_input_stream {
  static constexpr bool res = false;
};

template <typename S>
struct is_s2s_input_stream<input_stream<S>> {
  static constexpr bool res = true;
};

template <typename S>
inline constexpr bool is_s2s_input_stream_v = is_s2s_input_stream<S>::res;

template <typename T>
concept s2s_input_stream_like = is_s2s_input_stream_v<T>;
} /* namespace s2s */

#endif /* _STREAM_WRAPPER_IMPL_HPP_ */
