#ifndef __STREAM_HPP__
#define __STREAM_HPP__


#include <concepts>
#include <expected>
#include <fstream>
#include "../error.hpp"


template <typename T>
concept std_read_trait = requires(T obj, char* dest_mem, std::streamsize size_to_read) {
  { obj.read(dest_mem, size_to_read) } -> std::convertible_to<std::istream&>;
};

template <typename T>
concept read_trait = requires(T obj, char* dest_mem, std::streamsize size_to_read) {
  { obj.read(dest_mem, size_to_read) } -> std::same_as<T&>;
};

template <typename T>
concept std_write_trait = requires(T obj, const char* src_mem, std::size_t size_to_read) {
  { obj.write(src_mem, size_to_read) } -> std::convertible_to<std::ostream&>;
};

template <typename T>
concept write_trait = requires(T obj, const char* src_mem, std::size_t size_to_read) {
  { obj.write(src_mem, size_to_read) } -> std::same_as<T&>;
};

template <typename T>
concept convertible_to_bool = requires(T obj) {
  { obj.operator bool() } -> std::same_as<bool>;
  { !obj } -> std::same_as<bool>;
};


// todo add operator bool, seekg, tellg, fail, bad, eof/s constaint

template <typename T>
concept writeable = std_write_trait<T> ||
                    write_trait<T>;

template <typename T>
concept readable = std_read_trait<T> ||
                   read_trait<T>;

template <typename T>
concept input_stream_like = readable<T> &&
                            convertible_to_bool<T>;

template <typename T>
concept output_stream_like = writeable<T> &&
                             convertible_to_bool<T>;


using buffer_result = std::expected<void, cast_error>;


// todo maybe split to input_stream and output_stream
template <input_stream_like T>
class input_stream {
private:
  T& buffer;

public:
  // delete copy constructor?
  // todo interfacing with endiannes specify copy from file
  template <std::endian endianness>
  constexpr auto read(char* dest_mem, std::size_t size_to_read) -> buffer_result {
    // eof = buffer_exhaustion
    // bad | fail = io_error
    if(!buffer.read(dest_mem, size_to_read))
      return std::unexpected(cast_error::buffer_exhaustion);
    return {};
  }
};


template <output_stream_like T>
class output_stream {
private:
  T& buffer;

public:
  // delete copy constructor?
  template <std::endian endianness>
  constexpr auto write(const char* src_mem, std::size_t size_to_read) -> buffer_result {
    // eof = buffer_exhaustion
    // bad | fail = io_error
    if(!buffer.write(src_mem, size_to_read))
      return std::unexpected(cast_error::buffer_exhaustion);
    return {};
  }
};


static_assert(std_read_trait<std::ifstream>);
static_assert(convertible_to_bool<std::ifstream>);

#endif /* __STREAM_HPP__ */
