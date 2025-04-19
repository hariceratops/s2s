#ifndef __STREAM_HPP__
#define __STREAM_HPP__


#include <concepts>
#include <expected>
#include "error.hpp"
#include "sc_type_traits.hpp"



enum cast_endianness {
  host = 0,
  foreign = 1
};


template <std::endian endianness>
constexpr cast_endianness deduce_byte_order() {
  if constexpr(std::endian::native == endianness) 
    return cast_endianness::host;
  else if constexpr(std::endian::native != endianness) 
    return cast_endianness::foreign;
}


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


using rw_result = std::expected<void, cast_error>;


// todo maybe split to input_stream and output_stream
template <input_stream_like stream>
class input_stream {
private:
  stream& s;

  template <typename T>
  constexpr auto read_native_impl(T& obj, std::size_t size_to_read) -> rw_result {
    if(!s.read(byte_addressof(obj), size_to_read)) {
      return std::unexpected(cast_error::buffer_exhaustion);
    }
    return {};
  }

  template <constant_sized_like T>
  constexpr auto read_native(T& obj, std::size_t size_to_read) -> rw_result {
    return read_native_impl(obj, size_to_read);   
  }

  template <variable_sized_buffer_like T>
  constexpr auto read_native(T& obj, std::size_t len_to_read) -> rw_result {
    obj.resize(len_to_read);
    return read_native_impl(obj, len_to_read * sizeof(T{}[0]));
  }

  template <trivial T>
  constexpr auto read_foreign_scalar(T& obj, std::size_t size_to_read) -> rw_result {
    auto res = read_native_impl(obj, size_to_read);
    if(res) {
      // todo rollout byteswap if freestanding compiler doesnt provide one
      obj = std::byteswap(obj);
      return {};
    }
    return res;
  }

  template <buffer_like T>
  constexpr auto read_foreign_buffer(T& obj, std::size_t len_to_read) -> rw_result {
    auto res = read_native(obj, len_to_read);
    if(res) {
      for(auto& elem: obj) 
        obj = std::byteswap(obj);
      return {};
    }
    return res;
  }

public:
  input_stream(stream& s): s(s) {}
  // todo delete copy constructor?
  input_stream(const input_stream&) = delete;

  template <std::endian endianness, typename T>
  constexpr auto read(T& obj, std::size_t N) -> rw_result {
    auto constexpr byte_order = deduce_byte_order<endianness>();
    if constexpr(byte_order == cast_endianness::host) {
      return read_native(obj, N); 
    } else if constexpr(byte_order == cast_endianness::foreign) {
      if constexpr(trivial<T>) {
        return read_foreign_scalar(obj, N);
      } else if constexpr(buffer_like<T>) {
        return read_foreign_buffer(obj, N);
      }
    }
  }
};


template <output_stream_like stream>
class output_stream {
private:
  stream& s;

public:
  // delete copy constructor?
  template <typename T>
  constexpr auto write(const char* src_mem, std::size_t size_to_read) -> rw_result {
    // eof = buffer_exhaustion
    // bad | fail = io_error
    if(!s.write(src_mem, size_to_read))
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
