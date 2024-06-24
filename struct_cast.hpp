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

void* to_void_ptr(fixed_string auto& obj) {
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

char* byte_addressof(fixed_string auto& obj) {
    return reinterpret_cast<char*>(obj.data());
}

// FieldBase implementation
template <fixed_string id, typename T, std::size_t size>
struct field_base {
    static constexpr auto field_id = id;
    using field_type = T;
    static constexpr std::size_t field_size = size;
};

// Field implementation
template <fixed_string id, typename T, std::size_t size, auto constraint = no_constraint{}> 
    requires (arithmetic<T> || FixedSizedBufferLike<T>) && StrictlyCallableWith<decltype(constraint), T>
struct field : field_base<id, T, size> {
    T value;

    template <typename U>
    void read(const unsigned char* buffer, std::size_t size_to_read) {
        assert(constraint(value));
        std::memcpy(to_void_ptr(value), buffer, size_to_read);
    }

    void read(const std::ifstream& ifs, std::size_t size_to_read) {
        assert(constraint(value));
        ifs.read(byte_addressof(value), size_to_read);
    }
};

// StructField implementation
template <fixed_string id, typename T>
    requires std::derived_from<T, struct_field_list<typename T::entry_field_t>>
struct struct_field : field_base<id, T, sizeof(T)> {
    T value;
};

// Field lookup metafunction
template <typename List, fixed_string Id>
struct field_lookup;

template <fixed_string Id, typename T, std::size_t size, auto constraint, typename... Rest>
struct field_lookup<struct_field_list<field<Id, T, size, constraint>, Rest...>, Id> {
    using type = field<Id, T, size, constraint>;
};

template <fixed_string Id, typename First, typename... Rest>
struct field_lookup<struct_field_list<First, Rest...>, Id> {
    using type = typename field_lookup<struct_field_list<Rest...>, Id>::type;
};

template <fixed_string Id>
struct field_lookup<struct_field_list<>, Id> {
    struct field_lookup_failed {};
    using type = field_lookup_failed;
};

// Metafunction to check if a type is a struct_field_list
template <typename T>
struct is_struct_field_list : std::false_type {};

template <typename... Entries>
struct is_struct_field_list<struct_field_list<Entries...>> : std::true_type {};

template <typename T>
constexpr bool is_struct_field_list_v = is_struct_field_list<T>::value;

// StructFieldList implementation
template <typename... Entries>
struct struct_field_list : Entries... {
    using entry_field_t = std::tuple<typename Entries::field_type...>;

    template <typename FieldAccessor>
    constexpr auto& operator[](FieldAccessor) {
        using Field = typename field_lookup<struct_field_list, FieldAccessor::id>::type;
        return static_cast<Field&>(*this).value;
    }

    template <typename FieldAccessor>
    constexpr const auto& operator[](FieldAccessor) const {
        using Field = typename field_lookup<struct_field_list, FieldAccessor::id>::type;
        return static_cast<const Field&>(*this).value;
    }
};

// FieldAccessor implementation
template <fixed_string Id>
struct field_accessor {
    static constexpr auto id = Id;
};

// UDL operator
template <fixed_string Id>
constexpr auto operator"" _f() {
    return field_accessor<Id>{};
}

// Struct cast function
template <typename... Fields>
void struct_cast(struct_field_list<Fields...>& field_list, const unsigned char* buffer) {
    std::size_t prefix_sum[sizeof...(Fields) + 1] = {0};
    std::size_t index = 0;

    ([&](auto& field) {
        using FieldType = std::decay_t<decltype(field)>;
        prefix_sum[index + 1] = prefix_sum[index] + FieldType::field_size;
        if constexpr (is_struct_field_list_v<FieldType>) {
            struct_cast(field.value, buffer + prefix_sum[index]);
        } else {
            field.read(reinterpret_cast<const char*>(buffer + prefix_sum[index]), FieldType::field_size);
        }
        ++index;
    }(static_cast<Fields&>(field_list)), ...);
}

template <typename... Fields>
void struct_cast(struct_field_list<Fields...>& field_list, std::ifstream& stream) {
    std::size_t prefix_sum[sizeof...(Fields) + 1] = {0};
    std::size_t index = 0;

    ([&](auto& field) {
        using FieldType = std::decay_t<decltype(field)>;
        prefix_sum[index + 1] = prefix_sum[index] + FieldType::field_size;
        if constexpr (is_struct_field_list_v<FieldType>) {
            struct_cast(field.value, stream);
        } else {
            field.read(stream, FieldType::field_size);
        }
        ++index;
    }(static_cast<Fields&>(field_list)), ...);
}

#endif // STRUCT_CAST_HPP
