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
    constexpr fixed_string(const char (&str)[N + 1]) {
        std::copy_n(str, N, value.data());
    }
    constexpr const char* data() const { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

// FixedSizedBufferLike concept
template <typename T>
concept FixedSizedBufferLike = requires(T t) {
    { std::tuple_size<T>::value } -> std::convertible_to<std::size_t>;
    { t.data() } -> std::convertible_to<const char*>;
};

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
template <fixed_string id, typename T, std::size_t size/*, auto constraint = no_constraint{}*/> 
    requires (arithmetic<T> || FixedSizedBufferLike<T>) /*&& StrictlyCallableWith<decltype(constraint), T>*/
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


// StructField implementation
template <fixed_string id, typename T>
  requires is_struct_field_list_v<T>
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
struct field_list {};

struct field_lookup_failed {};

template <typename ls, fixed_string id>
struct field_lookup;

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

template <typename ls, fixed_string id>
using field_lookup_v = typename field_lookup<ls, id>::type;


// StructFieldList implementation
template <typename... fields>
struct struct_field_list : fields... {
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
