#ifndef _BYTES_HPP_
#define _BYTES_HPP_

#include <cstddef>  // for std::size_t
#include <climits>  // for CHAR_BIT

class byte_count {
public:
  constexpr explicit byte_count(std::size_t byte_size) 
  : size(byte_size) {}

  constexpr operator std::size_t() const {
    return size * CHAR_BIT; 
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size * CHAR_BIT);
  }

  constexpr byte_count operator+(const byte_count& other) const {
    return byte_count(size + other.size);
  }

  std::size_t size;
};

class bit_count {
public:
  constexpr explicit bit_count(std::size_t bit_size) 
  : size(bit_size) {}

  constexpr bit_count operator+(const bit_count& other) const {
    return bit_count(size + other.size);
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size);
  }

  std::size_t size;
};

// User-defined literal for byte_count
constexpr byte_count operator"" _B(unsigned long long int value) {
  return byte_count(static_cast<std::size_t>(value));
}

// User-defined literal for bit_count
constexpr bit_count operator"" _b(unsigned long long int value) {
  return bit_count(static_cast<std::size_t>(value));
}

namespace static_test {
  // Using user-defined literals to specify bytes and bits
  // 10 bytes
  constexpr auto byte_count_value = 10_B;
  // 32 bits
  constexpr auto bit_count_value = 32_b;

  static_assert(byte_count_value.size == 10);
  static_assert(bit_count_value.size == 32);

  // Demonstrating conversion from bytes to bits
  constexpr std::size_t bits_from_bytes = static_cast<std::size_t>(byte_count_value);
  static_assert(bits_from_bytes == 80);

  // Adding two bit_count objects
  // Adding 8 bits
  constexpr auto total_bits = bit_count_value + 8_b;
  static_assert(total_bits == 40);

  // Adding two byte_count objects
  // Adding 5 bytes
  constexpr auto total_bytes = byte_count_value + 5_B;
  static_assert(total_bytes.size == 15);

  // Using bit_count with shift operations
  // Shift left by 32 bits
  constexpr unsigned long long bit_shifted_value = 1ULL << static_cast<unsigned long long>(bit_count_value);
  static_assert(bit_shifted_value == 4294967296);

  // Using byte_count with shift operations
  constexpr unsigned long long byte_shifted_value = 1ULL << static_cast<unsigned long long>(9);
  static_assert(byte_shifted_value == 512);
}

#endif // _BYTES_HPP_
