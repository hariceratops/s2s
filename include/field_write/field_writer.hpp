#ifndef _FIELD_WRITER_HPP_
#define _FIELD_WRITER_HPP_

#include <expected>

#include "../field/field_traits.hpp"
#include "../field/field_metafunctions.hpp"
#include "../field_size/comptime_field_size_deduce.hpp"
#include "../field_size/field_size_deduce.hpp"
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

  template <auto endianness, typename stream>
  constexpr auto write(stream& s) const -> rw_result {
    using field_size = typename T::field_size;
    if constexpr(is_computed_size_v<field_size>) {
      // An arbitrary N-ary callable has no inverse, so its source fields stay
      // ordinary data and the size they imply can only be checked against the
      // container, never used to repair it.
      if(deduce_field_size<field_size>{}(field_list) != field.value.size())
        return std::unexpected(error_reason::found_contradicting_length);
    }
    // For a len_from_field size there is nothing to check: the length slot was
    // derived from this very container, so the container is the authority.
    return write_impl<endianness>(s, field.value, field.value.size());
  }
};


template <typename F, typename stream, auto endianness>
struct struct_write_impl;

// The one seam where the error representation narrows. A nested list names
// its own failing field, but rw_result carries no name and the outer fold
// re-attaches the outer field's id, so failed_at ends up naming the outermost
// record field. read_field<struct_field_like> does exactly the same.
template <field_list_like L, auto endianness, typename stream>
constexpr auto write_nested(stream& s, const L& nested) -> rw_result {
  auto res = struct_write_impl<L, stream, endianness>{}(s, nested);
  if(!res)
    return std::unexpected(res.error().failure_reason);
  return {};
}

template <struct_field_like T, field_list_like F>
struct write_field<T, F> {
  const T& field;
  const F& field_list;

  constexpr write_field(const T& field, const F& field_list)
    : field(field), field_list(field_list) {}

  template <auto endianness, typename stream>
  constexpr auto write(stream& s) const -> rw_result {
    using field_list_t = extract_type_from_field_v<T>;
    return write_nested<field_list_t, endianness>(s, field.value);
  }
};

template <array_of_record_field_like T, field_list_like F>
struct write_field<T, F> {
  const T& field;
  const F& field_list;

  constexpr write_field(const T& field, const F& field_list)
    : field(field), field_list(field_list) {}

  template <auto endianness, typename stream>
  constexpr auto write(stream& s) const -> rw_result {
    using array_type = typename T::field_type;
    using element_t = extract_type_from_array_v<array_type>;
    constexpr auto array_len = extract_size_from_array_v<array_type>;

    for(std::size_t count = 0; count < array_len; ++count) {
      auto res = write_nested<element_t, endianness>(s, field.value[count]);
      if(!res)
        return res;
    }
    return {};
  }
};

template <vector_of_record_field_like T, field_list_like F>
struct write_field<T, F> {
  const T& field;
  const F& field_list;

  constexpr write_field(const T& field, const F& field_list)
    : field(field), field_list(field_list) {}

  template <auto endianness, typename stream>
  constexpr auto write(stream& s) const -> rw_result {
    using vector_type = typename T::field_type;
    using element_t = extract_type_from_vec_t<vector_type>;
    using field_size = typename T::field_size;

    if constexpr(is_computed_size_v<field_size>) {
      if(deduce_field_size<field_size>{}(field_list) != field.value.size())
        return std::unexpected(error_reason::found_contradicting_length);
    }
    for(const auto& record: field.value) {
      auto res = write_nested<element_t, endianness>(s, record);
      if(!res)
        return res;
    }
    return {};
  }
};
} /* namespace s2s */

#endif // _FIELD_WRITER_HPP_
