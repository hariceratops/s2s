#include <print>
#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>
#include <sstream>
#include "../include/read.hpp"
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
  constexpr std::size_t num_elements = 30000;
  std::string binary_data = create_binary_data(num_elements);
  std::istringstream stream(binary_data);
  for (auto _ : state) {
    stream.seekg(0);
    std::vector<uint32_t> result(num_elements);
    stream.read(reinterpret_cast<char*>(result.data()), num_elements * sizeof(uint32_t));

    for (auto& val : result) {
      val = swap_bytes(val);
    }
    benchmark::DoNotOptimize(result);
  }
}

void bench_reverse_copy(benchmark::State& state) {
  constexpr std::size_t num_elements = 30000;
  std::string binary_data = create_binary_data(num_elements);
  std::istringstream stream(binary_data);

  for (auto _ : state) {
    stream.seekg(0);
    std::vector<uint32_t> result(num_elements);
    stream.read(reinterpret_cast<char*>(result.data()), num_elements * sizeof(uint32_t));

    for (auto& val : result) {
      std::reverse(reinterpret_cast<char*>(&val), reinterpret_cast<char*>(&val) + sizeof(uint32_t));
    }
    benchmark::DoNotOptimize(result);
  }
}

// Reads using foreign iterator
void bench_native_iterator(benchmark::State& state) {
  constexpr std::size_t num_elements = 30000;
  std::string binary_data = create_binary_data(num_elements);
  std::istringstream stream(binary_data);

  for (auto _ : state) {
    stream.seekg(0);
    std::vector<uint32_t> result(num_elements);

    raw_bytes wrapped_data(result);
    auto it = wrapped_data.fbegin();
    auto end = wrapped_data.fend();
    auto stream_it = std::istreambuf_iterator<char>(stream);
    auto stream_end = std::istreambuf_iterator<char>();

    while (it != end && stream_it != stream_end) {
        *it = *stream_it;
        ++it;
        ++stream_it;
    }
    benchmark::DoNotOptimize(result);
  }
}

// Reads using foreign iterator
void bench_foreign_iterator(benchmark::State& state) {
  constexpr std::size_t num_elements = 30000;
  std::string binary_data = create_binary_data(num_elements);
  std::istringstream stream(binary_data);

  for (auto _ : state) {
    stream.seekg(0);
    std::vector<uint32_t> result(num_elements);

    raw_bytes wrapped_data(result);
    auto it = wrapped_data.fbegin();
    auto end = wrapped_data.fend();
    auto stream_it = std::istreambuf_iterator<char>(stream);
    auto stream_end = std::istreambuf_iterator<char>();

    while (it != end && stream_it != stream_end) {
        *it = *stream_it;
        ++it;
        ++stream_it;
    }
    benchmark::DoNotOptimize(result);
  }
}

auto main(void) -> int {
  // static_assert(detect_compiler() == "gcc");
  uint32_t value = 0xdeadbeef;
  std::println("{}", swap_bytes(value));
  return 0;
}

