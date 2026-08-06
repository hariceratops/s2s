#ifndef _DERIVED_VALUE_HPP_
#define _DERIVED_VALUE_HPP_


#include <expected>

#include "../error/cast_error.hpp"
#include "../field/field.hpp"
#include "../field_list/field_list.hpp"
#include "../field_size/field_size.hpp"
#include "../field_size/comptime_field_size_deduce.hpp"


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

template <typename target, typename F>
struct is_derived_target {
  static constexpr bool value = false;
};

template <typename target, auto metadata, typename... fields>
struct is_derived_target<target, struct_field_list_impl<metadata, fields...>> {
  static constexpr bool value = (... || obligates<fields, target>());
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

    if(!agreed || !fits_declared_width(derived))
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
