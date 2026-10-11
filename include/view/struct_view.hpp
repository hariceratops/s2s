#ifndef _STRUCT_VIEW_HPP_
#define _STRUCT_VIEW_HPP_

#include <array>
#include <bit>
#include <cstddef>
#include <expected>
#include <span>
#include <type_traits>
#include <utility>
#include "../cast/struct_cast_impl.hpp"
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

// A function cannot return a C array, so those come back as the equivalent
// std::array.
template <typename T>
struct view_value {
  using type = T;
};

template <typename T, std::size_t N>
struct view_value<T[N]> {
  using type = std::array<T, N>;
};

template <typename S, std::endian order, raw_byte_like B>
class struct_view;

// A nested record is not decoded: it comes back as a view of the nested schema.
template <typename F, std::endian order, raw_byte_like B>
struct view_read {
  using type = const typename view_value<typename F::field_type>::type;
};

template <struct_field_like F, std::endian order, raw_byte_like B>
struct view_read<F, order, B> {
  using type = const struct_view<typename F::field_type, order, B>;
};

template <typename F, std::endian order, raw_byte_like B>
using view_read_t = typename view_read<F, order, B>::type;

template <typename F, std::endian order, raw_byte_like B>
constexpr auto read_leaf_at(const B* at) -> view_read_t<F, order, B> {
  auto field = decode_field_at<F, order>(at);
  if constexpr(std::is_array_v<typename F::field_type>)
    return std::to_array(field.value);
  else
    return field.value;
}


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

  template <typename, std::endian, raw_byte_like>
  friend class struct_view;

public:
  template <
    typename accessor,
    auto res = lookup_field<metadata>(as_sv(accessor::field_id))
  >
    requires (res.has_value)
  constexpr auto operator[](accessor) const -> view_read_t<meta::type_of<res->id>, order, B> {
    using field_t = meta::type_of<res->id>;
    const B* at = base + layout::offsets[res->occurs_at_idx];
    if constexpr(struct_field_like<field_t>)
      return struct_view<typename field_t::field_type, order, B>{at};
    else
      return read_leaf_at<field_t, order>(at);
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
    if constexpr(any_field_constrained_v<S>) {
      auto violated = first_violated_field(bytes.data());
      if(violated != layout::field_count)
        return std::unexpected(cast_error{error_reason::validation_failure, layout::names[violated]});
    }
    return struct_view<S, order, std::remove_const_t<B>>{bytes.data()};
  }

private:
  template <typename, std::endian>
  friend struct struct_view_impl;

  // Same predicate the parser applies, to a value decoded the parser's way. An
  // unconstrained field is not decoded at all.
  template <typename F, raw_byte_like B>
  static constexpr auto violates(const B* at) -> bool {
    if constexpr(struct_field_like<F>) {
      using nested = typename F::field_type;
      if constexpr(field_is_constrained_v<F>) {
        // A constraint over the whole record needs the whole record, so the
        // parser itself decodes it.
        unchecked_byte_source<B> source{{}, at};
        auto order_cell = deduce_byte_order<order>();
        auto record = struct_cast_impl<nested, unchecked_byte_source<B>>{}(source, order_cell);
        return !record || !F::constraint_checker(*record);
      } else if constexpr(any_field_constrained_v<nested>) {
        using nested_impl = struct_view_impl<nested, order>;
        return nested_impl::first_violated_field(at) != nested_impl::layout::field_count;
      } else {
        return false;
      }
    } else if constexpr(field_is_constrained_v<F>) {
      return !F::constraint_checker(decode_field_at<F, order>(at).value);
    } else {
      return false;
    }
  }

  // Wire order, stopping at the first violation; field_count when there is none.
  template <raw_byte_like B>
  static constexpr auto first_violated_field(const B* base) -> std::size_t {
    return first_violated_field(std::index_sequence_for<fields...>{}, base);
  }

  template <std::size_t... idx, raw_byte_like B>
  static constexpr auto first_violated_field(std::index_sequence<idx...>, const B* base) -> std::size_t {
    std::size_t found = layout::field_count;
    static_cast<void>(((violates<fields>(base + layout::offsets[idx]) ? (found = idx, true) : false) || ...));
    return found;
  }
};
} /* namespace s2s */

#endif // _STRUCT_VIEW_HPP_
