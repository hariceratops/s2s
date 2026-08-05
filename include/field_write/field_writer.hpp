#ifndef _FIELD_WRITER_HPP_
#define _FIELD_WRITER_HPP_

#include <expected>

#include "../field/field_traits.hpp"
#include "../field_size/comptime_field_size_deduce.hpp"
#include "../error/cast_error.hpp"
#include "write_impl.hpp"


namespace s2s {
template <typename F, typename L>
struct write_field;

template <fixed_sized_field_like T, field_list_like F>
struct write_field<T, F> {
  const T& field;
  const F& field_list;

  constexpr write_field(const T& field, const F& field_list)
    : field(field), field_list(field_list) {}

  template <auto endianness, typename stream>
  constexpr auto write(stream& s) const -> rw_result {
    using field_size = typename T::field_size;
    constexpr auto size_to_write = deduce_field_size<field_size>{}();
    return write_impl<endianness>(s, field.value, size_to_write);
  }
};
} /* namespace s2s */

#endif // _FIELD_WRITER_HPP_
