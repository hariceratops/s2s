#ifndef _FIELD_METAFUNCTIONS_HPP_
#define _FIELD_METAFUNCTIONS_HPP_

#include "fixed_string.hpp"
#include "field_size.hpp"
#include "field.hpp"


namespace s2s {
struct not_a_field;

template <typename T>
struct extract_type_from_field;

template <fixed_string id, typename field_type, typename size, auto constraint>
struct extract_type_from_field<field<id, field_type, field_size<size>, constraint>> {
  using type = field_type;
};

template <typename T>
struct extract_type_from_field {
  using type = not_a_field;
};

template <typename T>
using extract_type_from_field_v = typename extract_type_from_field<T>::type;
} /* namespace s2s */

#endif // _FIELD_METAFUNCTIONS_HPP_
