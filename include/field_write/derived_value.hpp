#ifndef _DERIVED_VALUE_HPP_
#define _DERIVED_VALUE_HPP_


#include <expected>

#include "../error/cast_error.hpp"
#include "../field/field.hpp"
#include "../field/field_traits.hpp"
#include "../field_list/field_list.hpp"
#include "../field_size/field_size.hpp"
#include "../field_size/comptime_field_size_deduce.hpp"
#include "../field_compute/computation_from_fields_impl.hpp"


namespace s2s {
// An obligation: a field whose data implies what an earlier field's value has
// to be. Only len_from_field is invertible — len_from_fields wraps an
// arbitrary callable, so it obligates nothing and its sources stay writable.
template <typename T>
struct len_obligation {
  static constexpr bool present = false;
};

template <fixed_string id, typename T, fixed_string len_source, auto constraint>
struct len_obligation<field<id, T, field_size<len_from_field<len_source>>, constraint>> {
  static constexpr bool present = true;
  static constexpr sv target = as_sv(len_source);
};

template <typename producer, typename target>
constexpr auto obligates() -> bool {
  if constexpr(len_obligation<producer>::present)
    return len_obligation<producer>::target == as_sv(target::field_id);
  else
    return false;
}

// A producer sitting inside a maybe_field obligates its target only when the
// optional is actually present. That is why such a target cannot be derived —
// there may be nothing to derive from — but when the producer is present its
// container still has to match whatever goes on the wire.
template <typename T>
struct conditional_len_obligation {
  static constexpr bool present = false;
};

template <fixed_string id, typename T, fixed_string len_source, auto constraint,
          typename present_only_if, typename optional>
struct conditional_len_obligation<
  maybe_field<
    field<id, T, field_size<len_from_field<len_source>>, constraint>,
    present_only_if,
    optional
  >
>
{
  static constexpr bool present = true;
  static constexpr sv target = as_sv(len_source);
};

template <typename producer, typename target>
constexpr auto conditionally_obligates() -> bool {
  if constexpr(conditional_len_obligation<producer>::present)
    return conditional_len_obligation<producer>::target == as_sv(target::field_id);
  else
    return false;
}

template <typename target, typename F>
struct has_conditional_len_obligation {
  static constexpr bool value = false;
};

template <typename target, auto metadata, typename... fields>
struct has_conditional_len_obligation<target, struct_field_list_impl<metadata, fields...>> {
  static constexpr bool value = (... || conditionally_obligates<fields, target>());
};

template <typename target, typename F>
inline constexpr bool has_conditional_len_obligation_v =
  has_conditional_len_obligation<target, F>::value;


template <typename target, typename F>
struct verify_conditional_len;

template <typename target, auto metadata, typename... fields>
struct verify_conditional_len<target, struct_field_list_impl<metadata, fields...>> {
  using S = struct_field_list_impl<metadata, fields...>;

  // An obligation counts only when its producer will actually be written.
  // Presence is judged by the same predicate the reader will apply, so a
  // predicate that disagrees with has_value() is left to fail at the optional
  // itself rather than being reported here as a length contradiction.
  constexpr auto operator()(const S& field_list, std::size_t value_on_the_wire) const
    -> rw_result
  {
    bool agreed{true};

    (([&] {
      if constexpr(conditionally_obligates<fields, target>()) {
        const auto& producer = static_cast<const fields&>(field_list);
        const auto is_active =
          producer.value.has_value() &&
          compute_impl<typename fields::field_presence_checker>{}(field_list);
        if(is_active && producer.value->size() != value_on_the_wire)
          agreed = false;
      }
    }()), ...);

    if(!agreed)
      return std::unexpected(error_reason::found_contradicting_length);
    return {};
  }
};


template <typename target, typename F>
struct is_derived_target {
  static constexpr bool value = false;
};

// Deliberately not an independent scan: operator[] rejects assignment to
// exactly the fields the writer overwrites, and two scans would drift.
template <typename target, auto metadata, typename... fields>
struct is_derived_target<target, struct_field_list_impl<metadata, fields...>> {
  static constexpr bool value = is_derived_field<metadata>(as_sv(target::field_id));

  // A length slot is always a fixed-width field. Without this a schema
  // pointing len_from_field at a variable-sized field fails deep inside
  // deduce_field_size with an incomplete-type error instead.
  static_assert(!value || fixed_sized_field_like<target>,
                "a derived length field must be a fixed-sized field");
};

template <typename target, typename F>
inline constexpr bool is_derived_target_v = is_derived_target<target, F>::value;


template <typename target, typename F>
struct derive_value;

// The obligations for a target all live in fields that come after it, and
// is_dependencies_resolved has already rejected any schema where that is not
// so — hence the forward scan over the whole pack.
template <typename target, auto metadata, typename... fields>
struct derive_value<target, struct_field_list_impl<metadata, fields...>> {
  using S = struct_field_list_impl<metadata, fields...>;
  using field_type = typename target::field_type;

  constexpr auto operator()(const S& field_list) const
    -> std::expected<field_type, error_reason>
  {
    std::size_t derived{0};
    bool seen{false};
    bool agreed{true};

    (([&] {
      if constexpr(obligates<fields, target>()) {
        auto implied = static_cast<const fields&>(field_list).value.size();
        if(seen && implied != derived)
          agreed = false;
        derived = implied;
        seen = true;
      }
    }()), ...);

    // Two distinct failures: the dependents contradict one another, or they
    // agree on a value the declared slot cannot hold.
    if(!agreed)
      return std::unexpected(error_reason::found_contradicting_length);
    if(!fits_declared_width(derived))
      return std::unexpected(error_reason::validation_failure);
    return static_cast<field_type>(derived);
  }

private:
  // The declared width, not sizeof(field_type): a u32 slot declared
  // field_size<fixed<2>> puts two bytes on the wire, and a length needing
  // three must fail rather than reach the stream truncated.
  static constexpr auto declared_width = deduce_field_size<typename target::field_size>{}();

  static constexpr auto fits_declared_width(std::size_t v) -> bool {
    if constexpr(declared_width < sizeof(std::size_t)) {
      if((v >> (declared_width * 8)) != 0)
        return false;
    }
    return static_cast<std::size_t>(static_cast<field_type>(v)) == v;
  }
};
} /* namespace s2s */

#endif // _DERIVED_VALUE_HPP_
