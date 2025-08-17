#ifndef _FIELD_LIST_LOOKUP_HPP_
#define _FIELD_LIST_LOOKUP_HPP_


#include <optional>
#include <string_view>
#include "../field/field_type_info.hpp"


namespace s2s {
using sv = std::string_view;

template <typename list_metadata>
constexpr auto lookup_field(sv field_name) -> std::optional<field_type_info> {
  auto field_table = list_metadata::field_table;
  return field_table[field_name];
}
}

#endif /* _FIELD_LIST_LOOKUP_HPP_ */
