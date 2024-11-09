#ifndef _FIELD_META_HPP_
#define _FIELD_META_HPP_

#include "fixed_string.hpp"
#include "field_size.hpp"
#include "field.hpp"


struct not_a_field;

template <typename T>
struct extract_type_from_field;

template <fixed_string id, typename field_type, typename size, auto constraint, auto present_only_if, auto type_deducer>
struct extract_type_from_field<field<id, field_type, field_size<size>, constraint, present_only_if, type_deducer>> {
  using type = field_type;
};

template <typename T>
struct extract_type_from_field {
  using type = not_a_field;
};

template <typename T>
using extract_type_from_field_v = typename extract_type_from_field<T>::type;

// todo: How to enable this test
// static_assert(std::is_same_v<extract_type_from_field_v<field<"x", int, 4>>, int>);
// static_assert(std::is_same_v<extract_type_from_field_v<field<"x", float, 4>>, float>);
// static_assert(std::is_same_v<extract_type_from_field_v<std::array<char, 10>>, not_a_field>);
// static_assert(
//   std::is_same_v<extract_type_from_field_v<
//     struct_field<"d", struct_field_list<field<"x", int, 4>, field<"y", int, 4>>>>, 
//     struct_field_list<field<"x", int, 4>, field<"y", int, 4>>>
//   );

#endif // _FIELD_META_HPP_
