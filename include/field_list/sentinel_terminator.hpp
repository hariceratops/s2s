#ifndef _SENTINEL_TERMINATOR_HPP_
#define _SENTINEL_TERMINATOR_HPP_


#include <type_traits>

#include "../field/field_accessor.hpp"
#include "../field_size/field_size.hpp"
#include "field_list.hpp"


namespace s2s {
// Cast the sentinel to the named field's type once, rather than promoting the
// field's value up: a signed char field holding 0xff compared against 0xff
// promotes to -1 == 255 and would never match the value that ends the run.
template <auto size, field_list_like record>
constexpr auto matches_sentinel(const record& r) -> bool {
  using sz = size_type_of<size>;
  const auto& v = field_value_of<field_accessor<sz::sentinel_field>>(r);
  return v == static_cast<std::remove_cvref_t<decltype(v)>>(sz::sentinel_value);
}
} /* namespace s2s */

#endif // _SENTINEL_TERMINATOR_HPP_
