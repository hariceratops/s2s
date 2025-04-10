#ifndef __STREAM_HPP__
#define __STREAM_HPP__


#include <concepts>
#include <expected>
#include "error.hpp"
#include "read.hpp"


// todo decide on how to wrap a user defined stream
// todo decide on constraints on streams
// todo is a wrapper needed, maybe constraint the copy function directly with stream traits

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


// todo add operator bool, seekg, tellg, fail, bad, eof/s constaint

template <typename T>
concept writeable = std_write_trait<T> || write_trait<T>;

template <typename T>
concept readable = std_read_trait<T> || read_trait<T>;

template <typename T>
concept input_stream_like = readable<T> && convertible_to_bool<T>;

template <typename T>
concept output_stream_like = writeable<T> && convertible_to_bool<T>;


using buffer_result = std::expected<void, cast_error>;


// todo maybe split to input_stream and output_stream
template <input_stream_like stream>
class input_stream {
private:
  stream& buffer;

  template <std::endian endianness, typename T>
  constexpr auto read(raw_bytes<T>& as_raw_bytes) -> buffer_result {
    auto streambuf_iter = std::istreambuf_iterator<char>(buffer);
    return copy_from_stream<endianness>(streambuf_iter, as_raw_bytes);
  }

public:
  input_stream(stream& buffer): buffer(buffer) {}
  // todo delete copy constructor?
  input_stream(const input_stream&) = delete;

  template <std::endian endianness, typename T>
  constexpr auto read(T& obj, std::size_t size_to_read) -> buffer_result {
    auto as_raw_bytes = raw_bytes<T>(obj, size_to_read);
    return read<endianness>(as_raw_bytes);
  }

  // todo handle read for fixed size array

  template <std::endian endianness, typename T>
  constexpr auto read(std::vector<T>& obj, std::size_t len_to_read) -> buffer_result {
    obj.resize(len_to_read);
    auto as_raw_bytes = raw_bytes<T>(obj);

    if constexpr(has_builtin_bswap<sizeof(T)>()) {
      auto streambuf_iter = std::istreambuf_iterator<char>(buffer);
      auto res = copy_from_stream(streambuf_iter, as_raw_bytes.begin(), as_raw_bytes.end(), obj.size);
      if(res) {
        for(auto& elem: obj)
          elem = swap_bytes(elem);
      }
      return res;
    } else {
      return read<endianness>(as_raw_bytes);
    }
  }
};


template <output_stream_like stream>
class output_stream {
private:
  stream& buffer;

public:
  // delete copy constructor?
  template <typename T>
  constexpr auto write(const char* src_mem, std::size_t size_to_read) -> buffer_result {
    // eof = buffer_exhaustion
    // bad | fail = io_error
    if(!buffer.write(src_mem, size_to_read))
      return std::unexpected(cast_error::buffer_exhaustion);
    return {};
  }
};

template <typename S>
struct is_s2s_input_stream;

template <typename S>
struct is_s2s_input_stream {
  static constexpr bool res = false;
};

template <typename S>
struct is_s2s_input_stream<input_stream<S>> {
  static constexpr bool res = true;
};

template <typename S>
inline constexpr bool is_s2s_input_stream_v = is_s2s_input_stream<S>::res;

template <typename T>
concept s2s_input_stream_like = is_s2s_input_stream_v<T>;

// static_assert(std_read_trait<std::ifstream>);
// static_assert(readable<std::ifstream>);
// static_assert(convertible_to_bool<std::ifstream>);

#endif /* __STREAM_HPP__ */
