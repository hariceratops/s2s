#ifndef _STREAM_CAST_HPP_
#define _STREAM_CAST_HPP_

#include <bit>
#include "../error/cast_error.hpp"
#include "../stream/stream_traits.hpp"
#include "../cast/stream_cast_impl.hpp"


namespace s2s {
template <field_list_like T, output_stream_like stream>
[[nodiscard]] constexpr auto stream_cast_le(stream& s, const T& obj) -> cast_result {
  return stream_cast_impl<T, stream, std::endian::little>{}(s, obj);
}

template <field_list_like T, output_stream_like stream>
[[nodiscard]] constexpr auto stream_cast_be(stream& s, const T& obj) -> cast_result {
  return stream_cast_impl<T, stream, std::endian::big>{}(s, obj);
}
} /* namespace s2s */

#endif // _STREAM_CAST_HPP_
