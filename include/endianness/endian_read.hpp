#include <fstream>
#include <iterator>
#include <algorithm>
#include <iostream>


template <typename T>
class mem_rep {
  char* mem;
  std::size_t size;

public:
  explicit constexpr mem_rep(T& obj, std::size_t size)
    : mem(reinterpret_cast<char*>(&obj)), size(size) {}

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
      bytewise_reverse_iterator temp = *this;
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

  bytewise_iterator begin() { return bytewise_iterator(mem); }
  bytewise_iterator end() { return bytewise_iterator(mem + size); }
  bytewise_reverse_iterator rbegin() { return bytewise_reverse_iterator(mem + size - 1); }
  bytewise_reverse_iterator rend() { return bytewise_reverse_iterator(mem - 1); }
};

