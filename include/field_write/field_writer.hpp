#ifndef _FIELD_WRITER_HPP_
#define _FIELD_WRITER_HPP_

#include <expected>
#include <utility>
#include <variant>

#include "../field/field_traits.hpp"
#include "../type_deduction/type/type_impl.hpp"
#include "../field/field_metafunctions.hpp"
#include "../field_size/comptime_field_size_deduce.hpp"
#include "../field_size/field_size_deduce.hpp"
#include "../field_compute/computation_from_fields_impl.hpp"
#include "../error/cast_error.hpp"
#include "derived_value.hpp"
#include "write_impl.hpp"


namespace s2s {
template <typename F, typename L>
struct write_field;

template <fixed_sized_field_like T, field_list_like F>
struct write_field<T, F> {
  const typename T::field_type& value;
  const F& field_list;

  constexpr write_field(const typename T::field_type& value, const F& field_list)
    : value(value), field_list(field_list) {}

  template <auto endianness, typename stream>
  constexpr auto write(stream& s) const -> rw_result {
    constexpr auto field_size = T::field_size;
    constexpr auto size_to_write = deduce_field_size<field_size>{}();
    if constexpr(is_derived_target_v<T, F>) {
      // The stored value is ignored, so the constraint has to be checked
      // against the derived one — stream_cast_impl skips this field.
      auto derived = derive_value<T, F>{}(field_list);
      if(!derived)
        return std::unexpected(derived.error());
      if(!T::constraint_checker(*derived))
        return std::unexpected(error_reason::validation_failure);
      return verify_then_write<endianness>(s, *derived, size_to_write);
    } else {
      return verify_then_write<endianness>(s, value, size_to_write);
    }
  }

private:
  // Conditional producers cannot make this field derived, so whatever value
  // reaches this point — derived or stored — still has to satisfy every
  // obligation that is currently active.
  template <auto endianness, typename stream>
  constexpr auto verify_then_write(
    stream& s, const typename T::field_type& v, std::size_t size_to_write) const -> rw_result
  {
    if constexpr(has_conditional_len_obligation_v<T, F>) {
      auto res = verify_conditional_len<T, F>{}(field_list, static_cast<std::size_t>(v));
      if(!res)
        return res;
    }
    return write_impl<endianness>(s, v, size_to_write);
  }
};

template <variable_sized_field_like T, field_list_like F>
struct write_field<T, F> {
  const typename T::field_type& value;
  const F& field_list;

  constexpr write_field(const typename T::field_type& value, const F& field_list)
    : value(value), field_list(field_list) {}

  template <auto endianness, typename stream>
  constexpr auto write(stream& s) const -> rw_result {
    constexpr auto field_size = T::field_size;
    if constexpr(is_computed_size_v<size_type_of<field_size>>) {
      // An arbitrary N-ary callable has no inverse, so its source fields stay
      // ordinary data and the size they imply can only be checked against the
      // container, never used to repair it.
      if(deduce_field_size<field_size>{}(field_list) != value.size())
        return std::unexpected(error_reason::found_contradicting_length);
    }
    // For a len_from_field size there is nothing to check: the length slot was
    // derived from this very container, so the container is the authority.
    return write_impl<endianness>(s, value, value.size());
  }
};


template <typename F, typename stream, auto endianness>
struct stream_cast_impl;

// The one seam where the error representation narrows. A nested list names
// its own failing field, but rw_result carries no name and the outer fold
// re-attaches the outer field's id, so failed_at ends up naming the outermost
// record field. read_field<struct_field_like> does exactly the same.
template <field_list_like L, auto endianness, typename stream>
constexpr auto write_nested(stream& s, const L& nested) -> rw_result {
  auto res = stream_cast_impl<L, stream, endianness>{}(s, nested);
  if(!res)
    return std::unexpected(res.error().failure_reason);
  return {};
}

template <struct_field_like T, field_list_like F>
struct write_field<T, F> {
  const typename T::field_type& value;
  const F& field_list;

  constexpr write_field(const typename T::field_type& value, const F& field_list)
    : value(value), field_list(field_list) {}

  template <auto endianness, typename stream>
  constexpr auto write(stream& s) const -> rw_result {
    using field_list_t = extract_type_from_field_v<T>;
    return write_nested<field_list_t, endianness>(s, value);
  }
};

template <array_of_record_field_like T, field_list_like F>
struct write_field<T, F> {
  const typename T::field_type& value;
  const F& field_list;

  constexpr write_field(const typename T::field_type& value, const F& field_list)
    : value(value), field_list(field_list) {}

  template <auto endianness, typename stream>
  constexpr auto write(stream& s) const -> rw_result {
    using array_type = typename T::field_type;
    using element_t = extract_type_from_array_v<array_type>;
    constexpr auto array_len = extract_size_from_array_v<array_type>;

    for(std::size_t count = 0; count < array_len; ++count) {
      auto res = write_nested<element_t, endianness>(s, value[count]);
      if(!res)
        return res;
    }
    return {};
  }
};

template <vector_of_record_field_like T, field_list_like F>
struct write_field<T, F> {
  const typename T::field_type& value;
  const F& field_list;

  constexpr write_field(const typename T::field_type& value, const F& field_list)
    : value(value), field_list(field_list) {}

  template <auto endianness, typename stream>
  constexpr auto write(stream& s) const -> rw_result {
    using vector_type = typename T::field_type;
    using element_t = extract_type_from_vec_t<vector_type>;
    constexpr auto field_size = T::field_size;

    if constexpr(is_computed_size_v<size_type_of<field_size>>) {
      if(deduce_field_size<field_size>{}(field_list) != value.size())
        return std::unexpected(error_reason::found_contradicting_length);
    }
    for(const auto& record: value) {
      auto res = write_nested<element_t, endianness>(s, record);
      if(!res)
        return res;
    }
    return {};
  }
};

template <optional_field_like T, field_list_like F>
struct write_field<T, F> {
  const typename T::field_type& value;
  const F& field_list;

  constexpr write_field(const typename T::field_type& value, const F& field_list)
    : value(value), field_list(field_list) {}

  template <auto endianness, typename stream>
  constexpr auto write(stream& s) const -> rw_result {
    // Presence is a predicate over siblings, not a stored flag, so there is
    // nothing to derive here — only to check that the struct agrees with what
    // the reader will conclude from the very same sibling bytes.
    const auto should_be_present =
      compute_impl<typename T::field_presence_checker>{}(field_list);
    if(should_be_present != value.has_value())
      return std::unexpected(error_reason::validation_failure);
    if(!should_be_present)
      return {};

    using base_t = typename T::field_base_type;
    // maybe_field rewrites the base field's constraint to no_constraint over
    // the optional, so the fold cannot run it and this is the only place it
    // reaches the engaged value.
    if(!base_t::constraint_checker(*value))
      return std::unexpected(error_reason::validation_failure);
    return write_field<base_t, F>(*value, field_list).template write<endianness>(s);
  }
};


template <std::size_t idx, typename E, typename F, typename V>
struct write_variant_impl {
  const V& variant;
  const F& field_list;

  constexpr write_variant_impl(const V& variant, const F& field_list)
    : variant(variant), field_list(field_list) {}

  template <auto endianness, typename stream>
  constexpr auto write(stream& s) const -> rw_result {
    if(variant.index() != idx)
      return {};
    const auto& alternative = std::get<idx>(variant);
    // The read side's reason applies here too: the fold only ever sees the
    // outer union_field, so the alternative's constraint has to be run
    // explicitly. It runs before write_field so a rejected payload leaves the
    // stream untouched — a discarded value is recoverable, half a record is not.
    if(!E::constraint_checker(alternative))
      return std::unexpected(error_reason::validation_failure);
    return write_field<E, F>(alternative, field_list).template write<endianness>(s);
  }
};

template <typename F, typename field_choices, typename idx_seq>
struct write_variant_helper;

template <typename F, typename... choices, std::size_t... idx>
struct write_variant_helper<F, field_choice_list<choices...>, std::index_sequence<idx...>> {
  template <auto endianness, typename stream, typename V>
  static constexpr auto write(stream& s, const V& variant, const F& field_list) -> rw_result {
    rw_result pipeline_seed{};
    return (
      pipeline_seed |
      ... |
      [&]() {
        return write_variant_impl<idx, choices, F, V>(variant, field_list)
                 .template write<endianness>(s);
      }
    );
  }
};

template <union_field_like T, field_list_like F>
struct write_field<T, F> {
  const typename T::field_type& value;
  const F& field_list;

  constexpr write_field(const typename T::field_type& value, const F& field_list)
    : value(value), field_list(field_list) {}

  template <auto endianness, typename stream>
  constexpr auto write(stream& s) const -> rw_result {
    using guide = typename T::type_deduction_guide;

    // Exactly the unions whose discriminant is derivable need no check here:
    // the discriminant came from this alternative, so agreement is structural.
    // A computed switch input or a ladder cannot be inverted, so the held
    // alternative can only be checked against what the reader will conclude
    // from the same sibling bytes.
    if constexpr(!discriminant_obligation<T>::present) {
      auto deduced = deduce_type<guide>{}(field_list);
      if(!deduced)
        return std::unexpected(deduced.error());
      if(*deduced != value.index())
        return std::unexpected(error_reason::validation_failure);
    }

    using helper = write_variant_helper<
      F,
      typename T::field_choices,
      std::make_index_sequence<T::variant_size>
    >;
    return helper::template write<endianness>(s, value, field_list);
  }
};
} /* namespace s2s */

#endif // _FIELD_WRITER_HPP_
