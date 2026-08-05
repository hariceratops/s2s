#ifndef _STRUCT_WRITE_HPP_
#define _STRUCT_WRITE_HPP_

#include <bit>
#include "../error/cast_error.hpp"
#include "../stream/stream_traits.hpp"
#include "../cast/struct_write_impl.hpp"


namespace s2s {
template <field_list_like T, output_stream_like stream>
[[nodiscard]] constexpr auto struct_write_le(stream& s, const T& obj) -> cast_result {
  return struct_write_impl<T, stream, std::endian::little>{}(s, obj);
}

template <field_list_like T, output_stream_like stream>
[[nodiscard]] constexpr auto struct_write_be(stream& s, const T& obj) -> cast_result {
  return struct_write_impl<T, stream, std::endian::big>{}(s, obj);
}
} /* namespace s2s */

#endif // _STRUCT_WRITE_HPP_
