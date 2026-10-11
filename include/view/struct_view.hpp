#ifndef _STRUCT_VIEW_HPP_
#define _STRUCT_VIEW_HPP_

#include <array>
#include <bit>
#include <cstddef>
#include <expected>
#include <span>
#include <type_traits>
#include "../error/cast_error.hpp"
#include "../field_list/field_list.hpp"
#include "../field_list/fixed_layout.hpp"
#include "../field_read/read_impl.hpp"
#include "../field_size/comptime_field_size_deduce.hpp"
#include "../stream/byte_order.hpp"
#include "../stream/stream_traits.hpp"


namespace s2s {
template <typename B>
concept raw_byte_like =
  std::is_same_v<std::remove_const_t<B>, char> ||
  std::is_same_v<std::remove_const_t<B>, unsigned char> ||
  std::is_same_v<std::remove_const_t<B>, std::byte>;

// Wrapped in an atom so gcc's diagnostic names the rule rather than printing
// nothing for a bare disjunction.
template <std::size_t extent, typename S>
inline constexpr bool span_covers_record_v =
  extent == std::dynamic_extent || extent >= record_layout<S>::width;

template <std::size_t extent, typename S>
concept span_covers_record = span_covers_record_v<extent, S>;


// Adapts a pointer to the constexpr-stream arm of read_impl, which is the one
// arm that works both during constant evaluation and at run time. Does not
// bounds-check: creation has already established the record is inside the span.
template <raw_byte_like B>
struct unchecked_byte_source : constexpr_stream {
  const B* at;

  template <std::size_t N>
  constexpr auto read(std::array<char, N>& dest, std::size_t size_to_read) -> unchecked_byte_source& {
    for(std::size_t idx = 0; idx < size_to_read; ++idx)
      dest[idx] = static_cast<char>(at[idx]);
    at += size_to_read;
    return *this;
  }

  explicit constexpr operator bool() const {
    return true;
  }
};

// Default-constructs F first, as the parser does, so a frozen field's seed is
// in place and narrow reads match it byte for byte.
template <typename F, std::endian order, raw_byte_like B>
constexpr auto decode_field_at(const B* at) -> F {
  F field;
  unchecked_byte_source<B> source{{}, at};
  // Cannot fail: creation placed every byte of the record inside the span.
  static_cast<void>(read_impl(source, field.value, deduce_field_size<F::field_size>{}(),
                              deduce_byte_order<order>()));
  return field;
}

template <typename F, std::endian order, raw_byte_like B>
using view_read_t = const typename F::field_type;


template <typename S, std::endian order, raw_byte_like B>
class struct_view;

template <typename S, std::endian order>
struct struct_view_impl;

template <auto metadata, typename... fields, std::endian order, raw_byte_like B>
class struct_view<struct_field_list_impl<metadata, fields...>, order, B> {
  using S = struct_field_list_impl<metadata, fields...>;
  using layout = record_layout<S>;

  const B* base;

  constexpr explicit struct_view(const B* first_byte) : base(first_byte) {}

  template <typename, std::endian>
  friend struct struct_view_impl;

public:
  template <
    typename accessor,
    auto res = lookup_field<metadata>(as_sv(accessor::field_id))
  >
    requires (res.has_value)
  constexpr auto operator[](accessor) const -> view_read_t<meta::type_of<res->id>, order, B> {
    using field_t = meta::type_of<res->id>;
    return decode_field_at<field_t, order>(base + layout::offsets[res->occurs_at_idx]).value;
  }
};

template <auto metadata, typename... fields, std::endian order>
struct struct_view_impl<struct_field_list_impl<metadata, fields...>, order> {
  using S = struct_field_list_impl<metadata, fields...>;
  using layout = record_layout<S>;

  template <raw_byte_like B, std::size_t extent>
  constexpr auto operator()(std::span<B, extent> bytes) const
      -> std::expected<struct_view<S, order, std::remove_const_t<B>>, cast_error> {
    if constexpr(extent == std::dynamic_extent) {
      if(bytes.size() < layout::width) {
        // A short span always has a first field whose end passes the span's end.
        std::size_t first_short = 0;
        while(layout::offsets[first_short] + layout::widths[first_short] <= bytes.size())
          ++first_short;
        return std::unexpected(cast_error{error_reason::buffer_exhaustion, layout::names[first_short]});
      }
    }
    return struct_view<S, order, std::remove_const_t<B>>{bytes.data()};
  }
};
} /* namespace s2s */

#endif // _STRUCT_VIEW_HPP_
