
#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>
#include <sstream>
#include "benchmark/benchmark.h"


template <std::size_t size>
constexpr bool has_builtin_bswap() {
  if constexpr (size == 2 || size == 4 || size == 8) 
  {
    #if defined(__GNC__) || defined(__clang__) || defined(_MSC_VER)
      return true;
    #else
      return false;
    #endif
  }
  return false;
}


template <typename T>
constexpr T swap_bytes(T value) {
  constexpr std::size_t size = sizeof(T);

  if constexpr(has_builtin_bswap<size>()) {
    #if defined(__GNUC__) || defined(__clang__)
      if constexpr (size == 2) return __builtin_bswap16(value);
      if constexpr (size == 4) return __builtin_bswap32(value);
      if constexpr (size == 8) return __builtin_bswap64(value);
    #elif defined(_MSC_VER)
      if constexpr (size == 2) return _byteswap_ushort(value);
      if constexpr (size == 4) return _byteswap_ulong(value);
      if constexpr (size == 8) return _byteswap_uint64(value);
    #endif
    } else {
    // Fallback: Reverse bytes manually
    T value_copy = value;
    uint8_t* ptr = reinterpret_cast<uint8_t*>(&value_copy);
    std::reverse(ptr, ptr + size);
    return value_copy;
  }
}

template <typename T>
char* byte_addressof(T& obj) {
  return reinterpret_cast<char*>(&obj);
}

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

std::string create_binary_data(std::size_t num_elements) {
  std::vector<uint32_t> data(num_elements);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);
  for (std::size_t i = 0; i < num_elements; ++i) {
      data[i] = dist(gen);
  }

  return std::string(reinterpret_cast<char*>(data.data()), data.size() * sizeof(uint32_t));
}

void bench_builtins(benchmark::State& state) {
  constexpr std::size_t num_elements = 1000;
  for (auto _ : state) {
    std::string src = create_binary_data(1000);
    std::istringstream stream(src);
    std::vector<uint32_t> dest;
    dest.resize(num_elements);

    for (std::size_t idx = 0; idx < num_elements; ++idx) {
      stream.read(reinterpret_cast<char*>(&dest[idx]), sizeof(uint32_t));
      dest[idx] = swap_bytes(dest[idx]);
    }
    benchmark::DoNotOptimize(dest);
  }
}

void bench_reverse_copy(benchmark::State& state) {
  constexpr std::size_t num_elements = 1000;
  for (auto _ : state) {
    std::string src = create_binary_data(1000);
    std::istringstream stream(src);
    std::vector<uint32_t> dest;
    dest.resize(num_elements);
    for (std::size_t idx = 0; idx < num_elements; ++idx) {
      stream.read(reinterpret_cast<char*>(&dest[idx]), sizeof(uint32_t));
      std::reverse(reinterpret_cast<char*>(&dest[idx]), reinterpret_cast<char*>(&dest[idx]) + sizeof(uint32_t));
    }
    benchmark::DoNotOptimize(dest);
  }
}

// Reads using foreign iterator
void bench_custom_iterator(benchmark::State& state) {
  constexpr std::size_t num_elements = 1000;

  for (auto _ : state) {
    std::string src = create_binary_data(1000);
    std::istringstream stream(src);
    std::vector<uint32_t> dest;
    dest.resize(num_elements);
    for (std::size_t idx = 0; idx < num_elements; ++idx) {
      raw_bytes<uint32_t> wrapped_data(dest[idx], 4);
      auto it = wrapped_data.rbegin();
      auto end = wrapped_data.rend();
      auto stream_it = std::istreambuf_iterator<char>(stream);
      auto stream_end = std::istreambuf_iterator<char>();
      auto b_count = 0;
      while (b_count < 4 && it != end && stream_it != stream_end) {
        *it = *stream_it;
        ++it;
        ++stream_it;
        b_count++;
      }
    }
    benchmark::DoNotOptimize(dest);
  }
}

BENCHMARK(bench_builtins);
BENCHMARK(bench_reverse_copy);
BENCHMARK(bench_custom_iterator);
