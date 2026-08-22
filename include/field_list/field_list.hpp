#ifndef _FIELD_LIST_HPP_
#define _FIELD_LIST_HPP_


#include "../field/field_type_info.hpp"
#include "../field/field_traits.hpp"
#include "field_list_base.hpp"
#include "field_list_metadata.hpp"
#include "../lib/metaprog/mp.hpp"


namespace s2s {

// A field whose value the write path derives from other fields. The name
// carries the reason into the diagnostic when an assignment is rejected.
template <typename field_accessor, auto list_metadata>
concept field_is_derived_from_other_fields =
  is_derived_field<list_metadata>(as_sv(field_accessor::field_id));

// A length target is not part of the user's model of the struct: nobody
// supplies it, and no assignment could keep it honest, since operator[] hands
// back a reference and a container can be mutated through it without passing
// any setter. So it has no operator[] overload at all.
template <typename field_accessor, auto list_metadata>
concept field_is_derived_from_a_length =
  is_length_derived_field<list_metadata>(as_sv(field_accessor::field_id));

template <typename field_accessor, auto list_metadata>
concept field_is_derived_from_a_discriminant =
  is_discriminant_derived_field<list_metadata>(as_sv(field_accessor::field_id));

// Unlike the two above, this is a property of the field alone rather than of
// what the rest of the list does with it, so it is answered from the looked-up
// type instead of from a list built over the whole pack.
template <auto list_metadata, typename field_accessor>
constexpr auto is_frozen_field() -> bool {
  constexpr auto res = lookup_field<list_metadata>(as_sv(field_accessor::field_id));
  if constexpr(res.has_value)
    return frozen_field_like<meta::type_of<res->id>>;
  else
    return false;
}

template <typename field_accessor, auto list_metadata>
concept field_is_frozen_by_its_constraint = is_frozen_field<list_metadata, field_accessor>();

// The two kinds that stay visible but refuse assignment. A length target is
// excluded even when it qualifies: it has no overload at all, and admitting one
// here would undo that.
template <typename field_accessor, auto list_metadata>
concept field_is_readable_but_not_assignable =
  (field_is_derived_from_a_discriminant<field_accessor, list_metadata> ||
   field_is_frozen_by_its_constraint<field_accessor, list_metadata>) &&
  (!field_is_derived_from_a_length<field_accessor, list_metadata>);

template <auto list_metadata, typename... fields>
struct struct_field_list_impl : struct_field_list_base, fields... {

  struct_field_list_impl() = default;

  // todo move as_sv to common place
  template <
    typename field_accessor,
    auto field_lookup_res = lookup_field<list_metadata>(as_sv(field_accessor::field_id))
  >
    requires (field_lookup_res.has_value) &&
             (!field_is_derived_from_other_fields<field_accessor, list_metadata>) &&
             (!field_is_frozen_by_its_constraint<field_accessor, list_metadata>)
  constexpr auto& operator[](field_accessor)  {
    using field_type_ref = meta::type_of<field_lookup_res->id>&;
    return static_cast<field_type_ref>(*this).value;
  }

  // A discriminant or a frozen field stays readable on a non-const object but
  // hands back a const reference, so an attempted assignment fails as
  // assign-to-const rather than as a wall of unsatisfied-constraint output from
  // no viable overload. Neither can be hidden outright the way a length target
  // is: a discriminant because how a caller should reach a variance field's
  // held alternative is unsettled, and a frozen field because what was parsed
  // off the wire is worth reading back even though it could only ever have been
  // the one value.
  template <
    typename field_accessor,
    auto field_lookup_res = lookup_field<list_metadata>(as_sv(field_accessor::field_id))
  >
    requires (field_lookup_res.has_value) &&
             field_is_readable_but_not_assignable<field_accessor, list_metadata>
  constexpr const auto& operator[](field_accessor) {
    using field_type_cref = const meta::type_of<field_lookup_res->id>&;
    return static_cast<field_type_cref>(*this).value;
  }

  // Constrained rather than unconditional: without excluding length targets
  // here, `const auto& n = fl["len"_f]` would still reach one through a const
  // object, and hiding it from the mutable overload alone would mean nothing.
  template <
    typename field_accessor,
    auto field_lookup_res = lookup_field<list_metadata>(as_sv(field_accessor::field_id))
  >
    requires (field_lookup_res.has_value) &&
             (!field_is_derived_from_a_length<field_accessor, list_metadata>)
  constexpr const auto& operator[](field_accessor) const {
    using field_type_cref = const meta::type_of<field_lookup_res->id>&;
    return static_cast<field_type_cref>(*this).value;
  }
};

// The lookup operator[] performs, without its visibility policy. The library's
// own read path resolves a len_from_field size and feeds user callables by
// reading fields, and both can name a length target — which the user cannot
// see, but which is still on the wire and still has to be read.
template <typename field_accessor, auto list_metadata, typename... fields>
constexpr auto& field_value_of(const struct_field_list_impl<list_metadata, fields...>& field_list) {
  constexpr auto field_lookup_res = lookup_field<list_metadata>(as_sv(field_accessor::field_id));
  static_assert(field_lookup_res.has_value, "no such field in this field list");
  using field_type_cref = const meta::type_of<field_lookup_res->id>&;
  return static_cast<field_type_cref>(field_list).value;
}
} /* namespace s2s */


#endif // _FIELD_LIST_HPP_
