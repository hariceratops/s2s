#ifndef __READ_HPP__
#define __READ_HPP__

#include <iterator>
#include <expected>
#include "error.hpp"


template <typename T>
struct raw_bytes {
  char* ptr_to_object;
  std::size_t size;

public:
  explicit constexpr raw_bytes(T& obj, std::size_t size)
    : ptr_to_object(reinterpret_cast<char*>(&obj)), size(size) {}

  // todo move iterators outside of class to decouple and improve compile time
  class bytewise_iterator {
    char* current;

  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using pointer = char*;
    using reference = char&;

    explicit constexpr bytewise_iterator(char* start)
      : current(start) {}
    char& operator*() { return *current; }
    bytewise_iterator& operator++() { ++current; return *this; }
    bytewise_iterator& operator++(int) {
      bytewise_iterator& temp = *this;
      ++current;
      return temp;
    }
    bool operator!=(const bytewise_iterator& other) const {
      return current != other.current;
    }
    bool operator==(const bytewise_iterator& other) const {
      return current == other.current;
    }
  };

  class bytewise_reverse_iterator {
    char* current;

  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using pointer = char*;
    using reference = char&;

    explicit constexpr bytewise_reverse_iterator(char* start)
      : current(start) {}
    char& operator*() { return *current; }
    bytewise_reverse_iterator& operator++() { --current; return *this;  }
    bytewise_reverse_iterator& operator++(int) {
      bytewise_reverse_iterator& temp = *this;
      --current;
      return temp;
    }
    bool operator!=(const bytewise_reverse_iterator& other) const {
      return current != other.current;
    }
    bool operator==(const bytewise_reverse_iterator& other) const {
      return current == other.current;
    }
  };

  bytewise_iterator begin() { return bytewise_iterator(ptr_to_object); }
  bytewise_iterator end() { return bytewise_iterator(ptr_to_object + size); }
  bytewise_reverse_iterator rbegin() { return bytewise_reverse_iterator(ptr_to_object + size - 1); }
  bytewise_reverse_iterator rend() { return bytewise_reverse_iterator(ptr_to_object - 1); }
};


enum cast_endianness {
  host = 0,
  foreign = 1
};

struct little_endian {};
struct big_endian {};


template <typename T>
concept std_input_stream_iter = requires(T obj, int _) {
  { obj.operator*() } -> std::same_as<char>;
  { obj.operator++() } -> std::same_as<T&>;
  { obj.operator++(_) } -> std::same_as<T&>;
  { std::is_same_v<typename T::iterator_category, std::input_iterator_tag> };
  { std::is_same_v<typename T::streambuf_type, std::basic_streambuf<char>> };
  { std::is_same_v<typename T::istream_type, std::basic_istream<char>> };
};

// todo is a custom input iterator needed
// using InputStreamIter = std::istreambuf_iterator<char>;
// todo if needed roll out one and right a concept for it
// template <typename T>
// concept input_stream_iter = requires(T obj, int _) {
//   { obj.operator*() } -> std::same_as<char>;
//   { obj.operator++() } -> std::same_as<T&>;
//   { obj.operator++(_) } -> std::same_as<T&>;
//   { std::is_same_v<typename T::iterator_category, std::input_iterator_tag> };
// };

// todo rename to copy and enclose in a namespace
template <typename InputStreamIter, typename ObjIter>
constexpr auto copy_from_stream(InputStreamIter& stream_iter, ObjIter&& obj_begin, ObjIter&& obj_end, std::size_t count) 
  -> std::expected<void, cast_error>  
{
  std::size_t b_count = 0;
  auto end_of_stream = InputStreamIter{};

  // todo out of bounds protection for obj_byte_iter
  while(b_count < count) {
    if(stream_iter == end_of_stream) 
      return std::unexpected(cast_error::buffer_exhaustion);
    if(obj_begin == obj_end)
      return std::unexpected(cast_error::buffer_exhaustion);

    *obj_begin = *stream_iter;
    obj_begin++;
    stream_iter++;
    b_count++;
  }
  return {};
}


template <std::endian endianness>
constexpr cast_endianness deduce_byte_order() {
  if constexpr(std::endian::native == endianness) return cast_endianness::host;
  else if constexpr(std::endian::native != endianness) return cast_endianness::foreign;
}


// todo decide if endianness should be in type or value domain
template <std::endian endianness, typename Iter, typename T>
constexpr auto copy_from_stream(Iter&& iter, raw_bytes<T>& obj) 
  -> std::expected<void, cast_error>
{
  constexpr auto byte_order = deduce_byte_order<endianness>();
  if constexpr(byte_order == cast_endianness::host) {
    return copy_from_stream(iter, obj.begin(), obj.end(), obj.size);
  } else if constexpr(byte_order == cast_endianness::foreign) {
    return copy_from_stream(iter, obj.rbegin(), obj.rend(), obj.size);
  }
}


template <typename InputStreamIter, typename ObjIter>
constexpr auto copy_vector_foreign(InputStreamIter& stream_iter, 
                                   ObjIter&& obj_begin, ObjIter&& obj_end, 
                                   std::size_t count_to_read, 
                                   std::size_t size_of_elem) 
  -> std::expected<void, cast_error>  
{
  std::size_t e_count = 0;
  std::size_t b_count = 0;
  auto end_of_stream = InputStreamIter{};
  
  while(e_count < count_to_read) {
    obj_begin += size_of_elem;

    if(obj_begin == obj_end)
      return std::unexpected(cast_error::buffer_exhaustion);

    // todo different error for input buffer exhaustion
    if(stream_iter == end_of_stream) 
      return std::unexpected(cast_error::buffer_exhaustion);

    while(b_count < size_of_elem) {
      *obj_begin = *stream_iter;
      obj_begin++;
      stream_iter++;
      b_count++;
    }
    e_count++;
  }
  return {};
}


// todo different raw_bytes implementation for non scalar types?
template <std::endian endianness, typename Iter, typename T>
constexpr auto copy_from_stream(Iter&& iter, raw_bytes<T>& obj, std::size_t count, std::size_t size_of_elem) 
  -> std::expected<void, cast_error>
{
  constexpr auto byte_order = deduce_byte_order<endianness>();
  if constexpr(byte_order == cast_endianness::host) {
    return copy_from_stream(iter, obj.begin(), obj.end(), obj.size);
  } else if constexpr(byte_order == cast_endianness::foreign) {
    return copy_vector_foreign(iter, obj.begin(), obj.end(), obj.size, count, size_of_elem);
  }
}

#endif /* __READ_HPP__ */
