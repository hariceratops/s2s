#ifndef _FIELD_LIST__HPP_
#define _FIELD_LIST__HPP_

#include "field_traits.hpp"
#include "field_lookup.hpp"


// template <fixed_string head>
// constexpr bool is_unique() { return true;  }
//
// template <fixed_string head, fixed_string... rest>
// constexpr bool is_unique() {
//   return ((head != rest) && ...) && is_unique(rest...);
// }
//
// static_assert(is_unique<"hello", "hello">());
// static_assert(is_unique<"hello", "world">());
//

template <typename... fields>
concept all_field_like = (field_like<fields> && ...);

template <typename... fields>
  requires all_field_like<fields...>
struct struct_field_list : struct_field_list_base, fields... {
  // todo: impl size resolution
  // todo: impl dependencies resolution
  // static_assert(size_indices_resolved_v<field_list<fields...>>, 
  //   "sizes not resolved. check if any of the fields which depends on the value of another field, \
  //    is always to the left of the dependant field and the field it depends on exists ");
  struct_field_list() = default;
  template <typename field_accessor, 
            typename field = field_lookup_v<field_list<fields...>, field_accessor::field_id>>
    requires (!std::is_same_v<field_lookup_failed, field>)
  constexpr auto& operator[](field_accessor)  {
    return static_cast<field&>(*this).value;
  }

  template <typename field_accessor,
            typename field = field_lookup_v<field_list<fields...>, field_accessor::field_id>>
    requires (!std::is_same_v<field_lookup_failed, field>)
  constexpr const auto& operator[](field_accessor) const {
    return static_cast<const field&>(*this).value;
  }
};

#endif // _FIELD_LIST__HPP_
