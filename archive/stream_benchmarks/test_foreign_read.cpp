#include <iostream>
#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>
#include <fstream>
#include <span>
#include <spanstream>

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

template <typename T>
char* byte_addressof(std::vector<T>& obj) {
  return reinterpret_cast<char*>(obj.data());
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

class foreign_vector_iterator {
public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = char;
  using difference_type = std::ptrdiff_t;
  using pointer = char*;
  using reference = char&;

private:
  pointer container_start;
  std::size_t element_size;
  std::size_t count_to_read;
  std::size_t global_byte_count;
  std::size_t local_byte_count;
  std::size_t element_read_count;
  std::size_t old_read_element_count;
  std::size_t element_start_pos;
  pointer current;

public:
  foreign_vector_iterator() : container_start(nullptr), current(nullptr) {}

  foreign_vector_iterator(char* container_start, std::size_t count_to_read, std::size_t element_size) :
      container_start(container_start),
      element_size(element_size),
      count_to_read(count_to_read),
      global_byte_count(1), 
      local_byte_count(1),
      element_read_count(0),
      old_read_element_count(0),
      element_start_pos(((element_read_count + 1) * element_size) - 1),
      current(container_start + element_start_pos) {}

  char& operator*() { return *current; }
  foreign_vector_iterator& operator++() {
    element_start_pos = ((element_read_count + 1) * element_size) - 1;
    if(element_start_pos > count_to_read) {
        current = nullptr;
        return *this;
    }
    current = container_start + element_start_pos - local_byte_count;
    global_byte_count++;
    local_byte_count++;
    element_read_count = global_byte_count / element_size;
    if(element_read_count != old_read_element_count) {
        old_read_element_count = element_read_count;
        local_byte_count = 0;
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
           foreign_vector_iterator(ptr_to_object, vector_size * element_size, element_size);
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
      std::cout << std::hex << temp << " ";
      file.write(reinterpret_cast<const char*>(&temp), sizeof(temp));
  }
  std::cout << '\n';
  file.close();
}

void create_binary_vector(std::ospanstream& stream, std::size_t num_elements) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);
  for (std::size_t i = 0; i < num_elements; ++i) {
      auto temp = dist(gen);
      std::cout << std::hex << temp << " ";
      stream.write(reinterpret_cast<const char*>(&temp), sizeof(temp));
  }
  std::cout << '\n';
}

void bench_builtins_scalar_from_file() {
    std::cout << __FUNCTION__ << '\n';
    constexpr std::size_t num_elements = 20;
    std::string file_name("test_builtins_scalar_from_file.bin");
    create_binary_data(file_name, num_elements);
    std::ifstream stream(file_name, std::ios::in | std::ios::binary);
    std::vector<uint32_t> dest;
    dest.resize(num_elements);

    for (std::size_t idx = 0; idx < num_elements; ++idx) {
        stream.read(reinterpret_cast<char*>(&dest[idx]), sizeof(uint32_t));
        dest[idx] = swap_bytes(dest[idx]);
    }
    for(auto val: dest) { std::cout << std::hex << val << " "; }
    std::cout << '\n';
}

void bench_reverse_copy_scalar_from_file() {
    std::cout << __FUNCTION__ << '\n';
    constexpr std::size_t num_elements = 20;
    std::string file_name("test_reverse_scalar_from_copy.bin");
    create_binary_data(file_name, num_elements);
    std::ifstream stream(file_name, std::ios::in | std::ios::binary);
    std::vector<uint32_t> dest;
    dest.resize(num_elements);

    for (std::size_t idx = 0; idx < num_elements; ++idx) {
      stream.read(reinterpret_cast<char*>(&dest[idx]), sizeof(uint32_t));
      std::reverse(reinterpret_cast<char*>(&dest[idx]), reinterpret_cast<char*>(&dest[idx]) + sizeof(uint32_t));
    }
    for(auto val: dest) { std::cout << std::hex << val << " "; }
    std::cout << '\n';
}


void bench_custom_iterator_scalar_from_file() {
    std::cout << __FUNCTION__ << '\n';
    constexpr std::size_t num_elements = 20;
    std::string file_name("test_custom_iterator_scalar_from_file.bin");
    create_binary_data(file_name, num_elements);
    std::ifstream stream(file_name, std::ios::in | std::ios::binary);
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
    for(auto val: dest) { std::cout << std::hex << val << " "; }
    std::cout << '\n';
}

void bench_builtins_vector_from_file() {
    std::cout << __FUNCTION__ << '\n';
    constexpr std::size_t num_elements = 20;
    std::string file_name("test_builtins_vector_from_file.bin");
    create_binary_data(file_name, num_elements);
    std::ifstream stream(file_name, std::ios::in | std::ios::binary);
    std::vector<uint32_t> dest;
    dest.resize(num_elements);

    stream.read(reinterpret_cast<char*>(dest.data()), num_elements * sizeof(uint32_t));
    for (std::size_t idx = 0; idx < num_elements; ++idx) {
        dest[idx] = swap_bytes(dest[idx]);
    }

    for(auto val: dest) { std::cout << std::hex << val << " "; }
    std::cout << '\n';
}

void bench_reverse_copy_vector_from_file() {
    std::cout << __FUNCTION__ << '\n';
    constexpr std::size_t num_elements = 20;
    std::string file_name("test_reverse_vector_from_copy.bin");
    create_binary_data(file_name, num_elements);
    std::ifstream stream(file_name, std::ios::in | std::ios::binary);
    std::vector<uint32_t> dest;
    dest.resize(num_elements);

    stream.read(reinterpret_cast<char*>(dest.data()), num_elements * sizeof(uint32_t));
    for (std::size_t idx = 0; idx < num_elements; ++idx) {
      std::reverse(reinterpret_cast<char*>(&dest[idx]), reinterpret_cast<char*>(&dest[idx]) + sizeof(uint32_t));
    }
    for(auto val: dest) { std::cout << std::hex << val << " "; }
    std::cout << '\n';
}


void bench_custom_iterator_vector_from_file() {
    std::cout << __FUNCTION__ << '\n';
    constexpr std::size_t num_elements = 20;
    std::string file_name("test_custom_iterator_vector_from_file.bin");
    create_binary_data(file_name, num_elements);
    std::ifstream stream(file_name, std::ios::in | std::ios::binary);
    std::vector<uint32_t> dest;
    dest.resize(num_elements);

    raw_bytes<std::vector<uint32_t>> wrapped_data(dest);
    auto it = wrapped_data.fbegin();
    auto end = wrapped_data.fend();
    auto stream_it = std::istreambuf_iterator<char>(stream);
    auto stream_end = std::istreambuf_iterator<char>();
    while (it != end && stream_it != stream_end) {
      *it = *stream_it;
      ++it;
      ++stream_it;
    }
    for(auto val: dest) { std::cout << std::hex << val << " "; }
    std::cout << '\n';
}

void bench_builtins_scalar_from_vector() {
    std::cout << __FUNCTION__ << '\n';
    constexpr std::size_t num_elements = 20;
    std::vector<char> buffer(num_elements * sizeof(uint32_t));
    std::ospanstream out_stream{std::span(buffer)};
    create_binary_vector(out_stream, num_elements);
    std::vector<uint32_t> dest;
    dest.resize(num_elements);
    std::ispanstream in_stream{std::span(buffer)};

    for (std::size_t idx = 0; idx < num_elements; ++idx) {
        in_stream.read(reinterpret_cast<char*>(&dest[idx]), sizeof(uint32_t));
        dest[idx] = swap_bytes(dest[idx]);
    }
    for(auto val: dest) { std::cout << std::hex << val << " "; }
    std::cout << '\n';
}

void bench_reverse_copy_scalar_from_vector() {
    std::cout << __FUNCTION__ << '\n';
    constexpr std::size_t num_elements = 20;
    std::vector<char> buffer(num_elements * sizeof(uint32_t));
    std::ospanstream out_stream{std::span(buffer)};
    create_binary_vector(out_stream, num_elements);
    std::vector<uint32_t> dest;
    dest.resize(num_elements);
    std::ispanstream in_stream{std::span(buffer)};

    for (std::size_t idx = 0; idx < num_elements; ++idx) {
        in_stream.read(reinterpret_cast<char*>(&dest[idx]), sizeof(uint32_t));
        std::reverse(reinterpret_cast<char*>(&dest[idx]), reinterpret_cast<char*>(&dest[idx]) + sizeof(uint32_t));
    }

    for(auto val: dest) { std::cout << std::hex << val << " "; }
    std::cout << '\n';
}

void bench_custom_iterator_scalar_from_vector() {
    std::cout << __FUNCTION__ << '\n';
    constexpr std::size_t num_elements = 20;
    std::vector<char> buffer(num_elements * sizeof(uint32_t));
    std::ospanstream out_stream{std::span(buffer)};
    create_binary_vector(out_stream, num_elements);
    std::vector<uint32_t> dest;
    dest.resize(num_elements);
    std::ispanstream in_stream{std::span(buffer)};

    for (std::size_t idx = 0; idx < num_elements; ++idx) {
        raw_bytes<uint32_t> wrapped_data(dest[idx], 4);
        auto it = wrapped_data.rbegin();
        auto end = wrapped_data.rend();
        auto stream_it = std::istreambuf_iterator<char>(in_stream);
        auto stream_end = std::istreambuf_iterator<char>();
        auto b_count = 0;
        while (b_count < 4 && it != end && stream_it != stream_end) {
            *it = *stream_it;
            ++it;
            ++stream_it;
            b_count++;
        }
    }
    for(auto val: dest) { std::cout << std::hex << val << " "; }
    std::cout << '\n';
}

void bench_builtins_vector_from_vector() {
    std::cout << __FUNCTION__ << '\n';
    constexpr std::size_t num_elements = 20;
    std::vector<char> buffer(num_elements * sizeof(uint32_t));
    std::ospanstream out_stream{std::span(buffer)};
    create_binary_vector(out_stream, num_elements);
    std::vector<uint32_t> dest;
    dest.resize(num_elements);
    std::ispanstream in_stream{std::span(buffer)};

    in_stream.read(reinterpret_cast<char*>(dest.data()), num_elements * sizeof(uint32_t));
    for (std::size_t idx = 0; idx < num_elements; ++idx) {
        dest[idx] = swap_bytes(dest[idx]);
    }
    for(auto val: dest) { std::cout << std::hex << val << " "; }
    std::cout << '\n';
}

void bench_reverse_copy_vector_from_vector() {
    std::cout << __FUNCTION__ << '\n';
    constexpr std::size_t num_elements = 20;
    std::vector<char> buffer(num_elements * sizeof(uint32_t));
    std::ospanstream out_stream{std::span(buffer)};
    create_binary_vector(out_stream, num_elements);
    std::vector<uint32_t> dest;
    dest.resize(num_elements);
    std::ispanstream in_stream{std::span(buffer)};

    in_stream.read(reinterpret_cast<char*>(dest.data()), num_elements * sizeof(uint32_t));
    for (std::size_t idx = 0; idx < num_elements; ++idx) {
        std::reverse(reinterpret_cast<char*>(&dest[idx]), reinterpret_cast<char*>(&dest[idx]) + sizeof(uint32_t));
    }
    for(auto val: dest) { std::cout << std::hex << val << " "; }
    std::cout << '\n';
}

void bench_custom_iterator_vector_from_vector() {
    std::cout << __FUNCTION__ << '\n';
    constexpr std::size_t num_elements = 20;

    std::vector<char> buffer(num_elements * sizeof(uint32_t));
    std::ospanstream out_stream{std::span(buffer)};
    create_binary_vector(out_stream, num_elements);
    std::vector<uint32_t> dest;
    dest.resize(num_elements);
    std::ispanstream in_stream{std::span(buffer)};

    raw_bytes<std::vector<uint32_t>> wrapped_data(dest);
    auto it = wrapped_data.fbegin();
    auto end = wrapped_data.fend();
    auto stream_it = std::istreambuf_iterator<char>(in_stream);
    auto stream_end = std::istreambuf_iterator<char>();
    while (it != end && stream_it != stream_end) {
      *it = *stream_it;
      ++it;
      ++stream_it;
    }
    for(auto val: dest) { std::cout << std::hex << val << " "; }
    std::cout << '\n';
}

auto main(void) -> int {
    bench_builtins_scalar_from_file();
    bench_reverse_copy_scalar_from_file();
    bench_custom_iterator_scalar_from_file();
    bench_builtins_vector_from_file();
    bench_reverse_copy_vector_from_file();
    bench_custom_iterator_vector_from_file();
    bench_builtins_scalar_from_vector();
    bench_reverse_copy_scalar_from_vector();
    bench_custom_iterator_scalar_from_vector();
    bench_builtins_vector_from_vector();
    bench_reverse_copy_vector_from_vector();
    bench_custom_iterator_vector_from_vector();
    return 0;
}
