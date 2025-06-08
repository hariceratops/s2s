#ifndef _STREAM_TRAITS_HPP_
#define _STREAM_TRAITS_HPP_

#include <array>
#include <concepts>
#include <iostream>


namespace s2s {

template <typename T>
concept convertible_to_bool = requires(T obj) {
  { obj.operator bool() } -> std::same_as<bool>;
  { !obj } -> std::same_as<bool>;
};

template <typename T>
concept std_read_trait = requires(T obj, char* dest_mem, std::streamsize size_to_read) {
  { obj.read(dest_mem, size_to_read) } -> std::convertible_to<std::istream&>;
};

template <typename T>
concept std_write_trait = requires(T obj, const char* src_mem, std::size_t size_to_read) {
  { obj.write(src_mem, size_to_read) } -> std::convertible_to<std::ostream&>;
};

template <typename T>
concept read_trait = requires(T obj, char* dest_mem, std::streamsize size_to_read) {
  { obj.read(dest_mem, size_to_read) } -> std::same_as<T&>;
};

template <typename T>
concept write_trait = requires(T obj, const char* src_mem, std::size_t size_to_read) {
  { obj.write(src_mem, size_to_read) } -> std::same_as<T&>;
};

template <typename T, std::size_t N>
concept constexpr_read_trait = requires(T obj, std::array<char, N>& dest_mem, std::streamsize size_to_read) {
  { obj.read(dest_mem, size_to_read) } -> std::same_as<T&>;
};

template <typename T, std::size_t N>
concept constexpr_write_trait = requires(T obj, const std::array<char, N>& src_mem, std::size_t size_to_read) {
  { obj.write(src_mem, size_to_read) } -> std::same_as<T&>;
};

// todo add operator bool, seekg, tellg, fail, bad, eof/s constaint
struct constexpr_stream{};

template <typename T>
concept identified_as_constexpr_stream = std::is_base_of_v<constexpr_stream, T>;

template <typename T>
concept writeable = std_write_trait<T> || write_trait<T>;

template <typename T>
concept readable = std_read_trait<T> || read_trait<T>;

// todo improve this to be more typesafe
template <typename T>
concept input_stream_like = (identified_as_constexpr_stream<T> || readable<T>) && convertible_to_bool<T>;

template <typename T>
concept output_stream_like = writeable<T> && convertible_to_bool<T>;
}

#endif /* _STREAM_TRAITS_HPP_ */
