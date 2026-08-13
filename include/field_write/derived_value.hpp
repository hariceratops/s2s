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

template <fixed_string id, typename T, auto size, auto constraint>
  requires (variable_size_like<size_type_of<size>> && !is_computed_size_v<size_type_of<size>>)
struct len_obligation<field<id, T, size, constraint>> {
  static constexpr bool present = true;
  static constexpr sv target = as_sv(len_source_of<size_type_of<size>>::value);
};

template <typename producer, typename target>
constexpr auto obligates() -> bool {
  if constexpr(len_obligation<producer>::present)
    return len_obligation<producer>::target == as_sv(target::field_id);
  else
    return false;
}

// The other invertible dependency. variant_from_type_conditions_v builds the
// variant in case order, so index i is case i positionally — the inverse is
// total, with no search and no ambiguity, and has_unique_match_values keeps
// it that way.
template <typename T>
struct discriminant_obligation {
  static constexpr bool present = false;
};

template <
  fixed_string id, fixed_string matched_id,
  template<typename...> typename type_switch,
  auto... match_values, typename... type_tags
>
struct discriminant_obligation<
  union_field<
    id,
    type<match_field<matched_id>, type_switch<match_case<match_values, type_tags>...>>
  >
>
{
  static constexpr bool present = true;
  static constexpr sv target = as_sv(matched_id);

  static constexpr auto value_at(std::size_t alternative_index) -> std::size_t {
    constexpr std::size_t values[] = {static_cast<std::size_t>(match_values)...};
    return values[alternative_index];
  }
};

template <typename producer, typename target>
constexpr auto discriminant_obligates() -> bool {
  if constexpr(discriminant_obligation<producer>::present)
    return discriminant_obligation<producer>::target == as_sv(target::field_id);
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

template <fixed_string id, typename T, auto size, auto constraint,
          typename present_only_if, typename optional>
  requires (variable_size_like<size_type_of<size>> && !is_computed_size_v<size_type_of<size>>)
struct conditional_len_obligation<
  maybe_field<
    field<id, T, size, constraint>,
    present_only_if,
    optional
  >
>
{
  static constexpr bool present = true;
  static constexpr sv target = as_sv(len_source_of<size_type_of<size>>::value);
};

template <typename producer, typename target>
constexpr auto conditionally_obligates() -> bool {
  if constexpr(conditional_len_obligation<producer>::present)
    return conditional_len_obligation<producer>::target == as_sv(target::field_id);
  else
    return false;
}

// The other conditional shape: a union alternative that is itself a
// length-prefixed container obligates its length only while that alternative
// is the one held.
template <typename producer, typename target, typename idx_seq>
struct union_len_obligation {
  static constexpr bool present = false;
};

template <typename target, typename... choices, std::size_t... idx>
struct union_len_obligation<field_choice_list<choices...>, target, std::index_sequence<idx...>> {
  static constexpr bool present = (... || obligates<choices, target>());

  template <typename V>
  static constexpr auto agrees(const V& variant, std::size_t value_on_the_wire) -> bool {
    bool ok{true};
    (([&] {
      if constexpr(obligates<choices, target>()) {
        if(variant.index() == idx && std::get<idx>(variant).size() != value_on_the_wire)
          ok = false;
      }
    }()), ...);
    return ok;
  }
};

template <typename producer, typename target>
struct union_len_obligation_of {
  static constexpr bool present = false;
};

template <fixed_string id, typename type_deducer, typename target>
struct union_len_obligation_of<union_field<id, type_deducer>, target> {
  using field = union_field<id, type_deducer>;
  using resolved = union_len_obligation<
    typename field::field_choices,
    target,
    std::make_index_sequence<field::variant_size>
  >;
  static constexpr bool present = resolved::present;
};


template <typename target, typename F>
struct has_conditional_len_obligation {
  static constexpr bool value = false;
};

template <typename target, auto metadata, typename... fields>
struct has_conditional_len_obligation<target, struct_field_list_impl<metadata, fields...>> {
  static constexpr bool value =
    (... || (conditionally_obligates<fields, target>() ||
             union_len_obligation_of<fields, target>::present));
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
      const auto& producer = static_cast<const fields&>(field_list);
      if constexpr(conditionally_obligates<fields, target>()) {
        const auto is_active =
          producer.value.has_value() &&
          compute_impl<typename fields::field_presence_checker>{}(field_list);
        if(is_active && producer.value->size() != value_on_the_wire)
          agreed = false;
      } else if constexpr(union_len_obligation_of<fields, target>::present) {
        using resolved = typename union_len_obligation_of<fields, target>::resolved;
        if(!resolved::agrees(producer.value, value_on_the_wire))
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

    auto record = [&](std::size_t implied) {
      if(seen && implied != derived)
        agreed = false;
      derived = implied;
      seen = true;
    };

    (([&] {
      const auto& producer = static_cast<const fields&>(field_list);
      if constexpr(obligates<fields, target>())
        record(producer.value.size());
      else if constexpr(discriminant_obligates<fields, target>())
        record(discriminant_obligation<fields>::value_at(producer.value.index()));
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
  static constexpr auto declared_width = deduce_field_size<target::field_size>{}();

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
