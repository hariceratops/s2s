#ifndef _API_STRUCT_VIEW_HPP_
#define _API_STRUCT_VIEW_HPP_

#include <bit>
#include <expected>
#include <span>
#include <type_traits>
#include "../error/cast_error.hpp"
#include "../field_list/fixed_layout.hpp"
#include "../view/struct_view.hpp"


namespace s2s {
template <fixed_layout_schema S, raw_byte_like B, std::size_t extent>
  requires span_covers_record<extent, S>
[[nodiscard]] constexpr auto struct_view_le(std::span<B, extent> bytes)
    -> std::expected<struct_view<S, std::endian::little, std::remove_const_t<B>>, cast_error> {
  return struct_view_impl<S, std::endian::little>{}(bytes);
}

template <fixed_layout_schema S, raw_byte_like B, std::size_t extent>
  requires span_covers_record<extent, S>
[[nodiscard]] constexpr auto struct_view_be(std::span<B, extent> bytes)
    -> std::expected<struct_view<S, std::endian::big, std::remove_const_t<B>>, cast_error> {
  return struct_view_impl<S, std::endian::big>{}(bytes);
}
} /* namespace s2s */

#endif // _API_STRUCT_VIEW_HPP_
