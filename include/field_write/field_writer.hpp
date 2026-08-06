#ifndef _FIELD_WRITER_HPP_
#define _FIELD_WRITER_HPP_

#include <expected>

#include "../field/field_traits.hpp"
#include "../field_size/comptime_field_size_deduce.hpp"
#include "../error/cast_error.hpp"
#include "derived_value.hpp"
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
    if constexpr(is_derived_target_v<T, F>) {
      // The stored value is ignored, so the constraint has to be checked
      // against the derived one — struct_write_impl skips this field.
      auto derived = derive_value<T, F>{}(field_list);
      if(!derived)
        return std::unexpected(derived.error());
      if(!T::constraint_checker(*derived))
        return std::unexpected(error_reason::validation_failure);
      return write_impl<endianness>(s, *derived, size_to_write);
    } else {
      return write_impl<endianness>(s, field.value, size_to_write);
    }
  }
};

template <variable_sized_field_like T, field_list_like F>
struct write_field<T, F> {
  const T& field;
  const F& field_list;

  constexpr write_field(const T& field, const F& field_list)
    : field(field), field_list(field_list) {}

  // The length is never consulted here: it was derived from this container
  // when the length field was written, so the container is the authority.
  template <auto endianness, typename stream>
  constexpr auto write(stream& s) const -> rw_result {
    return write_impl<endianness>(s, field.value, field.value.size());
  }
};
} /* namespace s2s */

#endif // _FIELD_WRITER_HPP_
