#ifndef _WRITE_IMPL_HPP_
#define _WRITE_IMPL_HPP_


#include <expected>
#include <bit>

#include "../error/cast_error.hpp"
#include "../lib/s2s_traits/type_traits.hpp"
#include "../lib/memory/address_manip.hpp"
#include "../stream/byte_order.hpp"


namespace s2s {
template <typename T, identified_as_constexpr_stream stream>
constexpr auto write_native_impl(stream& s, const T& obj, std::size_t size_to_write) -> rw_result {
  auto as_byte_buffer_rep = as_byte_buffer<stream>(obj);
  if(!s.write(as_byte_buffer_rep, size_to_write)) {
    return std::unexpected(error_reason::buffer_exhaustion);
  }
  return {};
}

template <typename T, writeable stream>
constexpr auto write_native_impl(stream& s, const T& obj, std::size_t size_to_write) -> rw_result {
  if(!s.write(const_byte_addressof<stream>(obj), size_to_write)) {
    return std::unexpected(error_reason::buffer_exhaustion);
  }
  return {};
}

template <constant_sized_like T, output_stream_like stream>
constexpr auto write_native(stream& s, const T& obj, std::size_t size_to_write) -> rw_result {
  return write_native_impl(s, obj, size_to_write);
}

template <variable_sized_buffer_like T, output_stream_like stream>
constexpr auto write_native(stream& s, const T& obj, std::size_t len_to_write) -> rw_result {
  if constexpr(identified_as_constexpr_stream<stream>) {
    // A vector's bytes cannot be bit_cast out of it during constant
    // evaluation, so the constexpr stream takes one element at a time.
    for(std::size_t idx = 0; idx < len_to_write; ++idx) {
      auto res = write_native_impl(s, obj[idx], sizeof(obj[idx]));
      if(!res)
        return res;
    }
    return {};
  } else {
    return write_native_impl(s, obj, len_to_write * sizeof(T{}[0]));
  }
}

template <trivial T, output_stream_like stream>
constexpr auto write_foreign_scalar(stream& s, const T& obj, std::size_t size_to_write) -> rw_result {
  // The source is const and belongs to the caller, so the swap lands in a
  // stack temporary rather than mutating it in place as the read path does.
  T swapped = std::byteswap(obj);
  return write_native_impl(s, swapped, size_to_write);
}

// The source buffer is const, so unlike the read direction there is nothing to
// swap in place. Writing element by element keeps the swap in a scalar
// temporary rather than staging a byteswapped copy of the whole buffer, which
// for a vector would mean allocating.
template <buffer_like T, output_stream_like stream>
constexpr auto write_foreign_buffer(stream& s, const T& obj, std::size_t len_to_write) -> rw_result {
  if constexpr(fixed_string_like<T>) {
    return write_native(s, obj, len_to_write);
  } else {
    for(const auto& elem: obj) {
      auto res = [&] {
        // Multi-dimensional aggregates nest, so descend until the scalars.
        if constexpr(buffer_like<std::remove_cvref_t<decltype(elem)>>)
          return write_foreign_buffer(s, elem, sizeof(elem));
        else if constexpr(sizeof(elem) > 1)
          return write_foreign_scalar(s, elem, sizeof(elem));
        else
          return write_native_impl(s, elem, sizeof(elem));
      }();
      if(!res)
        return res;
    }
    return {};
  }
}

template <std::endian endianness, typename T, output_stream_like stream>
constexpr auto write_impl(stream& s, const T& obj, std::size_t N) -> rw_result {
  auto constexpr byte_order = deduce_byte_order<endianness>();
  if constexpr(byte_order == cast_endianness::host) {
    return write_native(s, obj, N);
  } else if constexpr(byte_order == cast_endianness::foreign) {
    if constexpr(trivial<T>) {
      return write_foreign_scalar(s, obj, N);
    } else if constexpr(buffer_like<T>) {
      return write_foreign_buffer(s, obj, N);
    }
  }
}
} /* namespace s2s */

#endif // _WRITE_IMPL_HPP_
