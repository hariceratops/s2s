#ifndef _STREAM_CAST_HPP_
#define _STREAM_CAST_HPP_

#include <bit>
#include "../error/cast_error.hpp"
#include "../stream/stream_traits.hpp"
#include "../cast/stream_cast_impl.hpp"


namespace s2s {
template <field_list_like T, output_stream_like stream>
[[nodiscard]] constexpr auto stream_cast_le(stream& s, const T& obj) -> cast_result {
  auto order = deduce_byte_order<std::endian::little>();
  return stream_cast_impl<T, stream>{}(s, obj, order);
}

template <field_list_like T, output_stream_like stream>
[[nodiscard]] constexpr auto stream_cast_be(stream& s, const T& obj) -> cast_result {
  auto order = deduce_byte_order<std::endian::big>();
  return stream_cast_impl<T, stream>{}(s, obj, order);
}
} /* namespace s2s */

#endif // _STREAM_CAST_HPP_
