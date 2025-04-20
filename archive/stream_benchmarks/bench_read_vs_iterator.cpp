#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>
#include <fstream>
#include <spanstream>
#include "benchmark/benchmark.h"

template <typename T>
char* byte_addressof(T& obj) {
  return reinterpret_cast<char*>(&obj);
}

template <typename T>
char* byte_addressof(std::vector<T>& obj) {
  return reinterpret_cast<char*>(obj.data());
}

template <typename T>
struct raw_bytes;

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

private:
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
    --current;
    if(current == element_end) {
      element_start += element_size;
      element_end += element_size;
      if(element_start >= container_end)
         current = nullptr;
      else 
        current = element_start;
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

template <typename T>
struct raw_bytes<std::vector<T>> {
  char* ptr_to_object;
  std::size_t vector_size;
  std::size_t element_size;

public:
  explicit constexpr raw_bytes(std::vector<T>& obj)
    : ptr_to_object(byte_addressof(obj)), vector_size(obj.size()), element_size(sizeof(std::vector<T>{}[0])) {}

  bytewise_iterator begin() { return bytewise_iterator(ptr_to_object); }
  bytewise_iterator end() { return bytewise_iterator(ptr_to_object + vector_size * element_size); }
  foreign_vector_iterator fbegin() {
    return vector_size == 0 ? 
           foreign_vector_iterator() :
           foreign_vector_iterator(ptr_to_object, ptr_to_object + vector_size * element_size + 1, element_size);
  }
  foreign_vector_iterator fend() { return foreign_vector_iterator(); }
};


void create_binary_data(std::string file_name, std::size_t num_elements) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);
  std::ofstream file(file_name, std::ios::out | std::ios::binary);
  for (std::size_t i = 0; i < num_elements; ++i) {
      auto temp = dist(gen);
      file.write(reinterpret_cast<const char*>(&temp), sizeof(temp));
  }
  file.close();
}


void bench_stream_read_scalar_from_file(benchmark::State& state) {
  std::size_t num_elements = state.range(0);

  for (auto _ : state) {
    std::string file_name("test_stream_read_scalar_from_file.bin");
    create_binary_data(file_name, num_elements);
    std::ifstream stream(file_name, std::ios::in | std::ios::binary);
    std::vector<uint32_t> dest;
    dest.resize(num_elements);

    for (std::size_t idx = 0; idx < num_elements; ++idx) {
      stream.read(reinterpret_cast<char*>(&dest[idx]), sizeof(uint32_t));
    }
    benchmark::DoNotOptimize(dest);
  }
}


void bench_stream_custom_iterator_scalar_from_file(benchmark::State& state) {
  std::size_t num_elements = state.range(0);

  for (auto _ : state) {
    std::string file_name("test_stream_custom_iterator_scalar_from_file.bin");
    create_binary_data(file_name, num_elements);
    std::ifstream stream(file_name, std::ios::in | std::ios::binary);

    std::vector<uint32_t> dest;
    dest.resize(num_elements);
    for (std::size_t idx = 0; idx < num_elements; ++idx) {
      raw_bytes<uint32_t> wrapped_data(dest[idx], 4);
      auto it = wrapped_data.begin();
      auto end = wrapped_data.end();
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

void bench_stream_read_vector_from_file(benchmark::State& state) {
  std::size_t num_elements = state.range(0);

  for (auto _ : state) {
    std::string file_name("test_stream_read_vector_from_file.bin");
    create_binary_data(file_name, num_elements);
    std::ifstream stream(file_name, std::ios::in | std::ios::binary);

    std::vector<uint32_t> dest;
    dest.resize(num_elements);

    stream.read(reinterpret_cast<char*>(dest.data()), num_elements * sizeof(uint32_t));

    benchmark::DoNotOptimize(dest);
  }
}


void bench_stream_custom_iterator_vector_from_file(benchmark::State& state) {
  std::size_t num_elements = state.range(0);

  for (auto _ : state) {
    std::string file_name("test_stream_custom_iterator_vector_from_file.bin");
    create_binary_data(file_name, num_elements);
    std::ifstream stream(file_name, std::ios::in | std::ios::binary);

    std::vector<uint32_t> dest;
    dest.resize(num_elements);
    raw_bytes<std::vector<uint32_t>> wrapped_data(dest);
    auto it = wrapped_data.begin();
    auto end = wrapped_data.end();
    auto stream_it = std::istreambuf_iterator<char>(stream);
    auto stream_end = std::istreambuf_iterator<char>();
    while (it != end && stream_it != stream_end) {
      *it = *stream_it;
      ++it;
      ++stream_it;
    }

    benchmark::DoNotOptimize(dest);
  }
}


//BENCHMARK(bench_stream_read_scalar_from_file)->Arg(1000)->Arg(10000)->Arg(50000)->Arg(75000)->Arg(100000);
//BENCHMARK(bench_stream_custom_iterator_scalar_from_file)->Arg(1000)->Arg(10000)->Arg(50000)->Arg(75000)->Arg(100000);
BENCHMARK(bench_stream_read_vector_from_file)->Arg(1000)->Arg(10000)->Arg(50000)->Arg(75000)->Arg(100000);
BENCHMARK(bench_stream_custom_iterator_vector_from_file)->Arg(1000)->Arg(10000)->Arg(50000)->Arg(75000)->Arg(100000);
