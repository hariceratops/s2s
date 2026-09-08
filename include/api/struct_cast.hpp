#ifndef _STRUCT_CAST_HPP_
#define _STRUCT_CAST_HPP_

// status: split to cast and cast impl

#include <expected>
#include <bit>
#include "../error/cast_error.hpp"
#include "../stream/stream_traits.hpp"
#include "../cast/struct_cast_impl.hpp"


namespace s2s {
template <field_list_like T, input_stream_like stream>
[[nodiscard]] constexpr auto struct_cast_le(stream& s) -> std::expected<T, cast_error> {
  auto order = deduce_byte_order<std::endian::little>();
  return struct_cast_impl<T, stream>{}(s, order);
}

template <field_list_like T, input_stream_like stream>
[[nodiscard]] constexpr auto struct_cast_be(stream& s) -> std::expected<T, cast_error> {
  auto order = deduce_byte_order<std::endian::big>();
  return struct_cast_impl<T, stream>{}(s, order);
}

// The entry point for a schema that decides its own byte order: it takes none,
// because the file supplies it. The seed only governs the announcing record's
// own fields, which are read before resolution and are required to be
// order-agnostic.
template <field_list_like T, input_stream_like stream>
[[nodiscard]] constexpr auto struct_cast(stream& s) -> std::expected<T, cast_error> {
  auto order = cast_endianness::host;
  return struct_cast_impl<T, stream>{}(s, order);
}
} /* namespace s2s */

#endif // _STRUCT_CAST_HPP_
