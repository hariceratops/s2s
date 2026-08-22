#ifndef _FIELD_TRAITS_HPP_
#define _FIELD_TRAITS_HPP_


#include "field.hpp"
#include "../field_size/field_size.hpp"
#include "../lib/s2s_traits/type_traits.hpp"


namespace s2s {
template <typename T>
struct is_fixed_sized_field;

// Specialization for field with fixed_size_like size
template <fixed_string id, field_containable T, auto size, auto constraint_on_value, auto bound>
  requires fixed_size_like<size_type_of<size>>
struct is_fixed_sized_field<field<id, T, size, constraint_on_value, bound>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_fixed_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_fixed_sized_field_v = is_fixed_sized_field<T>::res;

template <typename T>
concept fixed_sized_field_like = is_fixed_sized_field_v<T>;

// A field the schema pins to one value: constrained to eq, so there is nothing
// for the user to decide and the write path supplies the value itself. Keyed
// off the constraint rather than off the descriptor alias, so magic_string,
// magic_number, magic_byte_array and a hand-spelled eq{} are all one case.
//
// Fixed-size only. A frozen std::string would make its own length slot
// statically known and collide with the derived-length machinery, and a frozen
// record is not a use case anyone has. The convertibility test excludes the
// remaining oddity, a c-array field: eq<char[N]> is a formable type but its
// value cannot be cast back to the array, so such a field keeps the ordinary
// validate-the-stored-value path rather than failing to compile here.
template <typename T>
concept frozen_field_like =
  fixed_sized_field_like<T> &&
  is_eq_constraint_v<std::remove_cvref_t<decltype(T::constraint_checker)>> &&
  std::is_convertible_v<decltype(T::constraint_checker.v), typename T::field_type>;

// Deliberately not sizeof(field_type): the same widening the write path applies,
// so a magic_number<"m", u64, 8_B, 0xdead> — whose eq holds an int, because the
// descriptor takes `auto expected` and eq deduces from the literal — reaches the
// stream as the field's own type.
template <frozen_field_like T>
inline constexpr auto frozen_value_of =
  static_cast<typename T::field_type>(T::constraint_checker.v);

template <typename T>
struct is_array_of_record_field;

template <fixed_string id, field_list_like T, std::size_t N, auto size, auto constraint_on_value, auto bound>
struct is_array_of_record_field<field<id, std::array<T, N>, size, constraint_on_value, bound>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_array_of_record_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_array_of_record_field_v = is_array_of_record_field<T>::res;

template <typename T>
concept array_of_record_field_like = is_array_of_record_field_v<T>;


template <typename T>
struct is_variable_sized_field;

// Specialization for field with variable_size_like size
template <fixed_string id, variable_sized_buffer_like T, auto size, auto constraint_on_value, auto bound>
  requires variable_size_like<size_type_of<size>>
struct is_variable_sized_field<field<id, T, size, constraint_on_value, bound>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_variable_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_variable_sized_field_v = is_variable_sized_field<T>::res;

// Concept for variable_sized_field_like
template <typename T>
concept variable_sized_field_like = is_variable_sized_field_v<T>;

template <typename T>
struct is_vector_of_record_field;

template <fixed_string id, field_list_like T, auto size, auto constraint_on_value, auto bound>
struct is_vector_of_record_field<field<id, std::vector<T>, size, constraint_on_value, bound>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_vector_of_record_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_vector_of_record_field_v = is_vector_of_record_field<T>::res;

template <typename T>
concept vector_of_record_field_like = is_vector_of_record_field_v<T>;

template <typename T>
struct is_struct_field;

// Specialization for field with variable_size_like size
template <fixed_string id, field_list_like T, auto size, auto constraint_on_value, auto bound>
struct is_struct_field<field<id, T, size, constraint_on_value, bound>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_struct_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_struct_field_v = is_struct_field<T>::res;

// Concept for variable_sized_field_like
template <typename T>
concept struct_field_like = is_struct_field_v<T>;


template <typename T>
struct is_optional_field;

// Specialization for maybe_field with a field
template <fixed_string id, 
          typename T, 
          auto size, 
          auto constraint, 
          auto bound,
          typename present_only_if, 
          typename optional>
struct is_optional_field<
    maybe_field<field<id, T, size, constraint, bound>, present_only_if, optional>
  >
{
  static constexpr bool res = true;
};

template <typename T>
struct is_optional_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_optional_field_v = is_optional_field<T>::res;

// Concept for optional_field_like
template <typename T>
concept optional_field_like = is_optional_field_v<T>;


template <typename T>
struct is_union_field;

template <fixed_string id, typename type_deducer, auto constraint_on_variant>
struct is_union_field<
    union_field<id, type_deducer, constraint_on_variant>
  >
{
  static constexpr bool res = true;
};

template <typename T>
struct is_union_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_union_field_v = is_union_field<T>::res;

template <typename T>
concept union_field_like = is_union_field_v<T>;

template <typename T>
concept field_like = fixed_sized_field_like<T> || 
                     variable_sized_field_like<T> ||
                     array_of_record_field_like<T> ||
                     vector_of_record_field_like<T> ||
                     struct_field_like<T> || 
                     optional_field_like<T> || 
                     union_field_like<T>;
} /* namespace s2s */

#endif /*_FIELD_TRAITS_HPP_*/
