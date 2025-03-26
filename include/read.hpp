#ifndef __READ_HPP__
#define __READ_HPP__

// todo compiler intrinsic byte swap for better performance


#include <iterator>
#include <expected>
#include "address_manip.hpp"
#include "error.hpp"
#include "sc_type_traits.hpp"


enum cast_endianness {
  host = 0,
  foreign = 1
};

struct little_endian {};
struct big_endian {};


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
  bytewise_iterator operator++(int) {
    bytewise_iterator temp = *this;
    ++(*this);
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
  bytewise_reverse_iterator operator++(int) {
    bytewise_reverse_iterator temp = *this;
    ++(*this);
    return temp;
  }
  bool operator!=(const bytewise_reverse_iterator& other) const {
    return current != other.current;
  }
  bool operator==(const bytewise_reverse_iterator& other) const {
    return current == other.current;
  }
};


class foreign_vector_iterator {
public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = char;
  using difference_type = std::ptrdiff_t;
  using pointer = char*;
  using reference = char&;

// private:
  pointer container_start;
  pointer container_end;
  std::size_t element_size;
  pointer element_start;
  pointer element_end;
  pointer current;

public:
  foreign_vector_iterator() :
    container_start(nullptr),
    container_end(nullptr),
    element_size(0),
    element_start(nullptr),
    element_end(nullptr),
    current(nullptr) {}

  foreign_vector_iterator(char* container_start, char* container_end, std::size_t element_size) 
    : container_start(container_start),
      container_end(container_end),
      element_size(element_size),
      element_start(container_start + element_size - 1),
      element_end(container_start - 1),
      current(element_start){}

  char& operator*() { return *current; }
  foreign_vector_iterator& operator++() { 
    if(current > element_end) { --current;  }
    else {
      if(element_start + element_size > container_end)
        current = element_start = element_end = nullptr;
      else {
        current = element_start = element_start + element_size;
        element_end = element_end + element_size;
      }
    }
    return *this;  
  }
  foreign_vector_iterator operator++(int) {
    foreign_vector_iterator temp = *this;
    ++(*this);
    return temp;
  }
  bool operator!=(const foreign_vector_iterator& other) const {
    return current != other.current;
  }
  bool operator==(const foreign_vector_iterator& other) const {
    return current == other.current;
  }
};


template <typename T>
struct raw_bytes;


template <typename T>
struct raw_bytes {
  char* ptr_to_object;
  std::size_t size;

public:
  explicit constexpr raw_bytes(T& obj, std::size_t size)
    : ptr_to_object(byte_addressof(obj)), size(size) {}

  bytewise_iterator begin() { return bytewise_iterator(ptr_to_object); }
  bytewise_iterator end() { return bytewise_iterator(ptr_to_object + size + 1); }
  bytewise_reverse_iterator rbegin() { return bytewise_reverse_iterator(ptr_to_object + size - 1); }
  bytewise_reverse_iterator rend() { return bytewise_reverse_iterator(ptr_to_object - 1); }
};


template <vector_like T>
struct raw_bytes<T> {
  char* ptr_to_object;
  std::size_t vector_size;
  std::size_t element_size;

public:
  explicit constexpr raw_bytes(T& obj)
    : ptr_to_object(byte_addressof(obj)), vector_size(obj.size()), element_size(sizeof(T{}[0])) {}

  bytewise_iterator begin() { return bytewise_iterator(ptr_to_object); }
  bytewise_iterator end() { return bytewise_iterator(ptr_to_object + vector_size * element_size); }
  foreign_vector_iterator fbegin() {
    return vector_size == 0 ? 
           foreign_vector_iterator() :
           foreign_vector_iterator(ptr_to_object, ptr_to_object + vector_size * element_size + 1, element_size);
  }
  foreign_vector_iterator fend() { return foreign_vector_iterator(); }
};


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
  // todo is this well defined
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


// todo optimization for word sized copy
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
