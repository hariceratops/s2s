#ifndef STRUCT_CAST_HPP
#define STRUCT_CAST_HPP

#include <array>
#include <cstddef>
#include <fstream>
#include <type_traits>
#include <cassert>
#include <concepts>
#include <cstring>
#include "value_constraints.hpp"

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

// Forward declaration
template <typename... fields>
struct struct_field_list;

// Metafunction to check if a type is a struct_field_list
template <typename T>
struct is_struct_field_list : std::false_type {};

template <typename... Entries>
struct is_struct_field_list<struct_field_list<Entries...>> : std::true_type {};

template <typename T>
constexpr bool is_struct_field_list_v = is_struct_field_list<T>::value;

template <typename T>
concept field_list_like = is_struct_field_list_v<T>;

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
struct is_fixed_string;

template <std::size_t N>
struct is_fixed_string<fixed_string<N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_string {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_string_v = is_fixed_string<T>::is_same;

template <typename T>
struct is_fixed_array;

template <typename T>
inline constexpr bool is_fixed_array_v = is_fixed_array<T>::is_same;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_fixed_array_v<T>)
struct is_fixed_array<std::array<T, N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_array {
  static constexpr bool is_same = false;
};

template <typename T>
struct is_c_array;

template <typename T>
inline constexpr bool is_c_array_v = is_c_array<T>::is_same;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_c_array_v<T>)
struct is_c_array<T[N]> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_c_array {
  static constexpr bool is_same = false;
};

// fixed_buffer_like concept
// todo constrain to array of primitives 
// todo check if array of records and arrays are possible for implementation
// todo check if md string is ok
template <typename T>
concept fixed_buffer_like = 
  is_fixed_array_v<T> ||
  is_c_array_v<T> ||
  is_fixed_string_v<T>;


template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

// Concept for strict callable
template <typename T, typename Arg>
concept StrictlyCallableWith = requires(T t, Arg arg) {
  { t(arg) } -> std::convertible_to<bool>;
} && std::is_same_v<Arg, typename std::remove_cvref_t<Arg>>;

// Function to void pointer cast
template <typename T>
void* to_void_ptr(T& obj) {
  return reinterpret_cast<void*>(&obj);
}

template <typename T, std::size_t N>
void* to_void_ptr(std::array<T, N>& obj) {
  return reinterpret_cast<void*>(obj.data());
}

template <std::size_t N>
void* to_void_ptr(fixed_string<N>& obj) {
  return reinterpret_cast<void*>(obj.data());
}

// Function to get byte address
template <typename T>
char* byte_addressof(T& obj) {
  return reinterpret_cast<char*>(&obj);
}

template <typename T, std::size_t N>
char* byte_addressof(std::array<T, N>& obj) {
  return reinterpret_cast<char*>(obj.data());
}

template <std::size_t N>
char* byte_addressof(fixed_string<N>& obj) {
  return reinterpret_cast<char*>(obj.data());
}

// FieldBase implementation
template <fixed_string id, typename T, std::size_t size>
struct field_base {
  static constexpr auto field_id = id;
  static constexpr std::size_t field_size = size;
  T value;
};


// Field implementation
template <fixed_string id, field_containable T, std::size_t size/*, auto constraint = no_constraint{}*/> 
struct field: public field_base<id, T, size> {
  void read(const char* buffer, std::size_t size_to_read) {
    std::memcpy(to_void_ptr(this->value), buffer, size_to_read);
    // assert(constraint(value));
  }

  void read(std::ifstream& ifs, std::size_t size_to_read) {
    ifs.read(byte_addressof(this->value), size_to_read);
    // assert(constraint(value));
  }
};



// StructField implementation
template <fixed_string id, field_list_like T>
struct struct_field : field_base<id, T, sizeof(T)> {};


// template <fixed_string id, typename field_type, std::size_t size>
// struct field;
struct not_a_field;

template <typename T>
struct extract_type_from_field;

template <fixed_string id, typename field_type, std::size_t size>
struct extract_type_from_field<field<id, field_type, size>> {
  using type = field_type;
};

template <fixed_string id, typename field_type>
struct extract_type_from_field<struct_field<id, field_type>> {
  using type = field_type;
};

template <typename T>
struct extract_type_from_field {
  using type = not_a_field;
};

template <typename T>
using extract_type_from_field_v = typename extract_type_from_field<T>::type;

static_assert(std::is_same_v<extract_type_from_field_v<field<"x", int, 4>>, int>);
static_assert(std::is_same_v<extract_type_from_field_v<field<"x", float, 4>>, float>);
static_assert(std::is_same_v<extract_type_from_field_v<std::array<char, 10>>, not_a_field>);
static_assert(
  std::is_same_v<extract_type_from_field_v<
    struct_field<"d", struct_field_list<field<"x", int, 4>, field<"y", int, 4>>>>, 
    struct_field_list<field<"x", int, 4>, field<"y", int, 4>>>
  );


// FieldAccessor implementation
template <fixed_string id>
struct field_accessor {
  static constexpr auto field_id = id;
};

// UDL operator
template <fixed_string id>
constexpr auto operator""_f() {
  return field_accessor<id>{};
}

// Field lookup metafunction
template <typename... ts>
struct field_list{};

struct field_lookup_failed {};
template <typename ls, fixed_string id>
struct field_lookup;
template <typename ls, fixed_string id>
using field_lookup_v = typename field_lookup<ls, id>::type;

template <typename... ts>
struct are_all_fields;
template <typename T>
inline constexpr bool are_all_fields_v = are_all_fields<T>::all_same;

// todo constraint all types shall be field_like
template <typename... fields>
struct struct_field_list : fields... {
  static_assert(are_all_fields_v<field_list<fields...>>, "struct_field_list shall be templated with field like types only");

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

template <fixed_string id, typename T, std::size_t size, typename... rest>
struct field_lookup<field_list<field<id, T, size>, rest...>, id> {
  using type = field<id, T, size>;
};

template <fixed_string id, typename T, typename... rest>
struct field_lookup<field_list<struct_field<id, T>, rest...>, id> {
  using type = struct_field<id, T>;
};

template <fixed_string id, typename head, typename... rest>
struct field_lookup<field_list<head, rest...>, id> {
  using type = typename field_lookup<field_list<rest...>, id>::type;
};

template <fixed_string id>
struct field_lookup<field_list<>, id> {
  using type = field_lookup_failed;
};

template <typename... ts>
struct are_all_fields;

template <>
struct are_all_fields<field_list<>> { static constexpr bool all_same = true; };

template <fixed_string id, field_containable T, std::size_t size>
struct are_all_fields<field_list<field<id, T, size>>> {
  static constexpr bool all_same = true;
};

template <fixed_string id, typename T>
struct are_all_fields<field_list<struct_field<id, T>>> {
  static constexpr bool all_same = true;
};

template <typename T, typename... rest>
struct are_all_fields<field_list<T, rest...>> {
  static constexpr bool all_same = false;
};

template <typename T>
struct are_all_fields<T> {
  static constexpr bool all_same = false;
};

template <fixed_string first_id, field_containable T, std::size_t size_first,
          typename... rest>
struct are_all_fields<field_list<field<first_id, T, size_first>, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};

template <fixed_string first_id, typename T, typename... rest>
struct are_all_fields<field_list<struct_field<first_id, T>, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};


static_assert(are_all_fields_v<field_list<field<"a", int, 4>>>);
static_assert(!are_all_fields_v<field_list<field<"a", int, 4>, int>>);
static_assert(!are_all_fields_v<field_list<int, int>>);
static_assert(are_all_fields_v<field_list<>>);
static_assert(are_all_fields_v<field_list<field<"a", int, 4>, field<"b", int, 4>>>);
static_assert(are_all_fields_v<field_list<field<"a", int, 4>, field<"b", fixed_string<4>, 4>>>);
static_assert(!are_all_fields_v<field_list<field<"a", int, 4>, field<"b", fixed_string<4>, 4>, float>>);


// Aliases
template <fixed_string id, typename T, std::size_t N>
using fixed_array_field = field<id, std::array<T, N>, N * sizeof(T)>;

template <fixed_string id, std::size_t N>
using fixed_string_field = field<id, fixed_string<N + 1>, N + 1>;

template <fixed_string id, typename T, std::size_t N>
using c_arr_field = field<id, T[N], N * sizeof(T)>;

template <fixed_string id, std::size_t N>
using c_str_field = field<id, char[N + 1], N * sizeof(char) + 1>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <fixed_string id, floating_point T>
using float_point_field = field<id, T, sizeof(T)>;


// Struct cast function
template <typename... fields>
constexpr void struct_cast(struct_field_list<fields...>& field_list, const unsigned char* buffer) {
  std::size_t prefix_sum[sizeof...(fields) + 1] = {0};
  std::size_t index = 0;

  ([&](auto& field) {
    using field_type = std::decay_t<decltype(field)>;
    prefix_sum[index + 1] = prefix_sum[index] + field_type::field_size;
    if constexpr (is_struct_field_list_v<extract_type_from_field_v<field_type>>) {
      struct_cast(field.value, buffer + prefix_sum[index]);
    } else {
      field.read(reinterpret_cast<const char*>(buffer + prefix_sum[index]), field_type::field_size);
    }
    ++index;
  }(static_cast<fields&>(field_list)), ...);
}

template <typename... fields>
constexpr void struct_cast(struct_field_list<fields...>& field_list, std::ifstream& stream) {
  ([&](auto& field) {
    using field_type = std::decay_t<decltype(field)>;
    if constexpr (is_struct_field_list_v<extract_type_from_field_v<field_type>>) {
      struct_cast(field.value, stream);
    } else {
      field.read(stream, field_type::field_size);
    }
  }(static_cast<fields&>(field_list)), ...);
}

#endif // STRUCT_CAST_HPP
