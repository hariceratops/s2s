#ifndef _FIELD_HPP_
#define _FIELD_HPP_


#include <variant>
#include <optional>

#include "../lib/containers/fixed_string.hpp"
#include "../field_size/field_size.hpp"
#include "../field_validation/field_value_constraints.hpp"
#include "../type_deduction/utils/type_condition_list.hpp"


namespace s2s {
// Whether a constraint pins this field to a value the field can actually hold.
// Kept as a trait rather than written inline in field's initializer: the
// convertibility test names eq's own member, which does not exist on any other
// constraint, and an unevaluated operand still has to be well-formed.
template <typename constraint_t, typename field_type>
struct is_seedable_eq {
  static constexpr bool res = false;
};

template <typename T, typename field_type>
struct is_seedable_eq<eq<T>, field_type> {
  static constexpr bool res = std::is_convertible_v<T, field_type>;
};

template <typename constraint_t, typename field_type>
inline constexpr bool is_seedable_eq_v = is_seedable_eq<constraint_t, field_type>::res;


template <fixed_string id,
          typename T,
          auto size,
          auto constraint_on_value,
          auto max_byte_bound = use_default_bound>
struct field {
  using field_type = T;
  static constexpr auto field_size = size;
  // The ceiling the read path applies before allocating this field. Defaulted,
  // so every construction site that predates bounds keeps compiling.
  static constexpr auto field_bound = max_byte_bound;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;

  // A frozen field's stored value is the value it writes. Without this it is
  // whatever default construction left behind, and every write-side computation
  // that reads such a field — a byte-order announcement's marker, a maybe's
  // presence predicate, a computed size, a ladder branch — sees a zero the
  // field can never actually hold.
  //
  // Seeded in the constructor rather than in a default member initializer: a
  // c-array field type cannot be returned from a function or copy-initialized
  // from one, and the assignment form lets the array case discard the branch
  // instead of failing to declare it. Such a field is never seeded anyway —
  // eq over a C array fails the convertibility test, the same oddity
  // frozen_field_like's own comment documents.
  constexpr field() {
    if constexpr(is_seedable_eq_v<std::remove_cvref_t<decltype(constraint_on_value)>, field_type>)
      value = static_cast<field_type>(constraint_on_value.v);
  }

  field_type value{};
};

template <typename T>
struct to_optional_field;

template <fixed_string id, typename T, auto size, auto constraint_on_value, auto bound>
struct to_optional_field<field<id, T, size, constraint_on_value, bound>> {
  using res = field<id, std::optional<T>, size, no_constraint<std::optional<T>>{}, bound>;
};

template <typename T>
using to_optional_field_v = to_optional_field<T>::res;

template <typename T>
struct no_variance_field;

template <fixed_string id,
          typename T,
          auto size,
          auto bound>
struct no_variance_field<field<id, T, size, no_constraint<T>{}, bound>> {
  static constexpr bool res = true;
};

template <typename T>
struct no_variance_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool no_variance_field_v = no_variance_field<T>::res;

template <typename T>
concept no_variance_field_like = no_variance_field_v<T>;




template <typename base_field,
          typename present_only_if,
          typename optional = to_optional_field_v<base_field>>
class maybe_field : public optional
{
public:
  using field_base_type = base_field;
  using field_presence_checker = present_only_if;
};


// A record whose bytes decide the byte order of everything read after it.
// Shaped like maybe_field — public inheritance from the base field plus two
// member aliases — so every trait, the field table and both folds go on seeing
// the base field. The guide stays an opaque typename here, which is what keeps
// this header's include closure to the three it already has.
template <typename base_field, typename order_deduction>
struct order_announcing_field : base_field {
  using field_base_type = base_field;
  using byte_order_deduction = order_deduction;
};


template <typename... choices>
struct field_choice_list {};

template <fixed_string id, typename... args>
struct to_field_choices;

// Matched on the case list rather than on a variant zipped against a parallel
// size list: an alternative's options travel on its own tag, so there is no
// second list to keep aligned — and no third one the next time an option is
// added. Every alternative takes the union's id, which is why a per-alternative
// failure reports the union in cast_error::failed_at.
template <fixed_string id, typename... cases>
struct to_field_choices<id, type_condition_list<cases...>> {
  using choices = field_choice_list<
    field<id,
          typename cases::type_tag::type,
          cases::type_tag::size,
          cases::type_tag::constraint,
          cases::type_tag::bound>...>;
};

// The constraint is over the resolved variant, not over an alternative's
// payload: an alternative states its own on its tag. Defaulted, so every
// construction site that predates the union-level one keeps its meaning — and
// the default may name type_deducer because it is an earlier parameter.
template <fixed_string id,
          typename type_deducer,
          auto constraint_on_variant = no_constraint<typename type_deducer::variant>{}>
struct union_field: public
    field<
      id,
      typename type_deducer::variant,
      size_dont_care,
      constraint_on_variant
    >
{
  using type_deduction_guide = type_deducer;
  static constexpr auto variant_size = std::variant_size_v<typename type_deducer::variant>;
  using field_choices = typename to_field_choices<
      id,
      typename type_deducer::conditions
    >::choices;
};

} /* namespace s2s */

#endif // _FIELD_HPP_
