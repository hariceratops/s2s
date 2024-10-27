#ifndef _CAST_HPP_
#define _CAST_HPP_

#include "field_reader.hpp"
#include "../field_meta.hpp"
#include "sc_type_traits.hpp"

template <typename T>
struct is_field;

template <typename T>
struct is_field: std::false_type{};

template <fixed_string id, typename T, typename field_size, auto constraint>
struct is_field<field<id, T, field_size, constraint>>: std::true_type{};

template <typename T>
constexpr bool is_field_v = is_field<T>::value;

template <typename T>
struct is_field_with_runtime_size;

template <typename T>
struct is_field_with_runtime_size: std::false_type{};

template <fixed_string id, typename T, typename field_size, auto constraint>
struct is_field_with_runtime_size<runtime_field<id, T, field_size, constraint>>: std::true_type{};

template <typename T>
constexpr bool is_field_with_runtime_size_v = is_field_with_runtime_size<T>::value;

static_assert(is_field_with_runtime_size_v<runtime_field<"hello", int, runtime_size<from_field<"a">>>>);
static_assert(!is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
static_assert(is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
static_assert(!is_field_v<runtime_field<"hello", int, runtime_size<from_field<"a">>>>);

template <typename... fields>
struct struct_field_list;

// Metafunction to check if a type is a struct_field_list
template <typename T>
struct is_struct_field_list : std::false_type {};

template <typename... Entries>
struct is_struct_field_list<struct_field_list<Entries...>> : std::true_type {};

template <typename T>
constexpr bool is_struct_field_list_v = is_struct_field_list<T>::value;

template <typename... expected_types>
auto is_any_error(const expected_types&... expected_list) {
  return (expected_list && ...);
}

// todo constraints
template <typename expected_struct_field_list, typename error>
auto operator|(const std::expected<expected_struct_field_list, error>& lhs, auto functor) {
  if(lhs) return functor(*lhs); else return lhs;
}

// todo constraints
template <typename field_list, typename field>
struct read_node {
  using result_type = std::expected<field_list, std::string>;
  using field_type = extract_type_from_field_v<field>;

  // todo how to bundle buf with the res. a new box will be helpful?
  std::ifstream buf;
  read_node(std::ifstream& buf): buf(buf) {}

  // todo: for const buf how to track the index in prefix sum
  // auto operator()(result_type input) -> result_type {}
};


template <typename... T>
void func(const T&... expected_list) {
    (([&]() {
        std::cout << typeid(T{}).name() << '\n';
    }()), ...);
}

template <typename... fields>
constexpr auto struct_cast(const unsigned char* buffer)
  -> std::expected<struct_field_list<fields...>, std::string>
{
  using field_list_type = struct_field_list<fields...>;
  using res_type = std::expected<field_list_type, std::string>;

  res_type res;
  std::size_t prefix_sum[sizeof...(fields) + 1] = {0};
  std::size_t index = 0;

  // return (res | read_node<field_list_type, fields>{} | ...);
  return (
    res | 
    [&](res_type input) -> res_type {
      auto field = static_cast<fields&>(input);
      using field_type = std::decay_t<decltype(field)>;
      if constexpr (is_struct_field_list_v<extract_type_from_field_v<field_type>>) {
        auto struct_read_res = struct_cast(field.value, buffer + prefix_sum[index]);
      } else if constexpr (is_field_v<field_type>) {
        field.value = read<field_type>(reinterpret_cast<const char*>(buffer + prefix_sum[index]), field_type::field_size);
      } else if constexpr (is_field_with_runtime_size_v<field_type>) {
        field.value = read<field_type>(reinterpret_cast<const char*>(buffer + prefix_sum[index]), res[field_type::field_accessor]);
      }
      // todo fix bug in case of updating the prefic sum for var len field
      prefix_sum[index + 1] = prefix_sum[index] + field_type::field_size;
      ++index;
    } | 
  ...);
}

template <typename... fields>
constexpr void struct_cast(struct_field_list<fields...>& field_list, std::ifstream& stream) {
  ([&](auto& field) {
    using field_type = std::decay_t<decltype(field)>;
    if constexpr (is_struct_field_list_v<extract_type_from_field_v<field_type>>) {
      struct_cast(field.value, stream);
    } else if constexpr (is_field_v<field_type>) {
      field.read(stream, field_type::field_size);
    } else if constexpr (is_field_with_runtime_size_v<field_type>) {
      field.read(stream, field_list[field_type::field_accessor]);
    }

  }(static_cast<fields&>(field_list)), ...);
}

#endif // _CAST_HPP_
