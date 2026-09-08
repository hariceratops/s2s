#ifndef _STREAM_CAST_HPP_
#define _STREAM_CAST_HPP_

#include <bit>
#include "../error/cast_error.hpp"
#include "../field_list/announcing_record.hpp"
#include "../stream/stream_traits.hpp"
#include "../cast/stream_cast_impl.hpp"


namespace s2s {
template <fixed_order_schema T, output_stream_like stream>
[[nodiscard]] constexpr auto stream_cast_le(stream& s, const T& obj) -> cast_result {
  auto order = deduce_byte_order<std::endian::little>();
  return stream_cast_impl<T, stream>{}(s, obj, order);
}

template <fixed_order_schema T, output_stream_like stream>
[[nodiscard]] constexpr auto stream_cast_be(stream& s, const T& obj) -> cast_result {
  auto order = deduce_byte_order<std::endian::big>();
  return stream_cast_impl<T, stream>{}(s, obj, order);
}

// No order argument, for the same reason struct_cast takes none: the struct's
// own marker decides. Whatever it holds — from a prior parse, or set by the
// caller — is the order everything after it is written in. There is no
// requested order to derive the marker from and none to check it against; the
// stored value is authoritative because there is nothing else.
template <self_announcing_schema T, output_stream_like stream>
[[nodiscard]] constexpr auto stream_cast(stream& s, const T& obj) -> cast_result {
  auto order = cast_endianness::host;
  return stream_cast_impl<T, stream>{}(s, obj, order);
}
} /* namespace s2s */

#endif // _STREAM_CAST_HPP_
