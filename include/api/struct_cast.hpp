#ifndef _STRUCT_CAST_HPP_
#define _STRUCT_CAST_HPP_

// status: split to cast and cast impl

#include <expected>
#include <bit>
#include "../error/cast_error.hpp"
#include "../field/field.hpp"
#include "../field_list/field_list.hpp"
#include "../stream/stream_traits.hpp"
#include "../cast/struct_cast_impl.hpp"


namespace s2s {
template <field_list_like T, input_stream_like stream>
[[nodiscard]] constexpr auto struct_cast_le(stream& s) -> std::expected<T, cast_error> {
  return struct_cast_impl<T, stream, std::endian::little>{}(s);
}

template <field_list_like T, input_stream_like stream>
[[nodiscard]] constexpr auto struct_cast_be(stream& s) -> std::expected<T, cast_error> {
  return struct_cast_impl<T, stream, std::endian::big>{}(s);
}
} /* namespace s2s */

#endif // _STRUCT_CAST_HPP_
