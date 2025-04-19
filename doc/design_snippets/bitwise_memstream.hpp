#ifndef _MEMSTREAM_HPP_
#define _MEMSTREAM_HPP_

#include <iostream>
#include <vector>
#include <array>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <optional>
#include <bitset>
#include <limits>
#include <climits>
#include <utility>
#include <type_traits>
#include "../error.hpp"

// Type for specifying a number of bytes to read/write
template <std::size_t N>
struct bytes_to_read {
  static constexpr std::size_t size = N;  // Number of bytes

  // Converting constructor to bits
  constexpr operator std::size_t() const {
    return size * CHAR_BIT;  // Convert bytes to bits
  }
};

// Type for specifying a number of bits to read/write
template <std::size_t N>
struct bits_to_read {
  static constexpr std::size_t size = N;  // Number of bits

  // Adding two bits_to_read objects
  template <std::size_t M>
  constexpr bits_to_read<size + M> operator+(bits_to_read<M> /*other*/) const {
    return bits_to_read<size + M>{};
  }
};

// User-defined literal for bytes_to_read
constexpr bytes_to_read<1> operator"" _B(unsigned long long int value) {
  return bytes_to_read<static_cast<std::size_t>(value)>{};
}

// User-defined literal for bits_to_read
constexpr bits_to_read<1> operator"" _b(unsigned long long int value) {
  return bits_to_read<static_cast<std::size_t>(value)>{};
}

// A tag to distinguish between memory and file streams
struct MemoryStreamTag {};
struct FileStreamTag {};

// Base class for handling different stream types
template <typename StreamType>
class StreamHandler {
public:
  // Read from a stream
  template <typename Container>
  static std::optional<std::size_t> read(Container& buffer, std::size_t& byte_pos, std::size_t& bit_pos, char* dest, std::size_t bits_to_read);

  // Write to a stream
  template <typename Container>
  static std::optional<std::size_t> write(Container& buffer, std::size_t& byte_pos, std::size_t& bit_pos, const char* src, std::size_t bits_to_write);

  // Seek to a specific position
  static std::optional<std::size_t> seek(std::size_t& byte_pos, std::size_t& bit_pos, std::size_t new_pos, std::size_t max_size);
};

// Specialization for memory streams
template <>
class StreamHandler<MemoryStreamTag> {
public:
  template <typename Container>
  static std::optional<std::size_t> read(Container& buffer, std::size_t& byte_pos, std::size_t& bit_pos, char* dest, std::size_t bits_to_read) {
    std::size_t total_bits = byte_pos * CHAR_BIT + bit_pos;
    std::size_t buffer_bits = buffer.size() * CHAR_BIT;

    if (total_bits + bits_to_read > buffer_bits) {
      return std::nullopt;  // Error: reading beyond buffer size
    }

    std::size_t bits_read = 0;
    std::size_t dest_index = 0;
    while (bits_read < bits_to_read) {
      std::size_t current_byte = byte_pos + (bit_pos + bits_read) / CHAR_BIT;
      std::size_t current_bit = (bit_pos + bits_read) % CHAR_BIT;
      std::size_t bits_to_copy = std::min(CHAR_BIT - current_bit, bits_to_read - bits_read);

      std::bitset<8> byte_bits(buffer[current_byte]);
      std::bitset<8> dest_bits(dest[dest_index]);

      dest_bits |= (byte_bits >> current_bit) & ((1 << bits_to_copy) - 1);

      dest[dest_index] = static_cast<char>(dest_bits.to_ulong());
      bits_read += bits_to_copy;
      if (bits_to_read - bits_read >= CHAR_BIT) {
        ++dest_index;
      }
    }

    byte_pos += (bit_pos + bits_read) / CHAR_BIT;
    bit_pos = (bit_pos + bits_read) % CHAR_BIT;
    return bits_read;
  }

  template <typename Container>
  static std::optional<std::size_t> write(Container& buffer, std::size_t& byte_pos, std::size_t& bit_pos, const char* src, std::size_t bits_to_write) {
    std::size_t total_bits = byte_pos * CHAR_BIT + bit_pos;
    std::size_t buffer_bits = buffer.size() * CHAR_BIT;

    if (total_bits + bits_to_write > buffer_bits) {
      return std::nullopt;  // Error: writing beyond buffer size
    }

    std::size_t bits_written = 0;
    std::size_t src_index = 0;
    while (bits_written < bits_to_write) {
      std::size_t current_byte = byte_pos + (bit_pos + bits_written) / CHAR_BIT;
      std::size_t current_bit = (bit_pos + bits_written) % CHAR_BIT;
      std::size_t bits_to_copy = std::min(CHAR_BIT - current_bit, bits_to_write - bits_written);

      std::bitset<8> src_bits(src[src_index]);
      std::bitset<8> byte_bits(buffer[current_byte]);

      byte_bits &= ~(((1 << bits_to_copy) - 1) << current_bit);  // Clear the bits to be written
      byte_bits |= (src_bits.to_ulong() & ((1 << bits_to_copy) - 1)) << current_bit;

      buffer[current_byte] = static_cast<char>(byte_bits.to_ulong());
      bits_written += bits_to_copy;
      if (bits_to_write - bits_written >= CHAR_BIT) {
        ++src_index;
      }
    }

    byte_pos += (bit_pos + bits_written) / CHAR_BIT;
    bit_pos = (bit_pos + bits_written) % CHAR_BIT;
    return bits_written;
  }

  static std::optional<std::size_t> seek(std::size_t& byte_pos, std::size_t& bit_pos, std::size_t new_pos, std::size_t max_size) {
    if (new_pos > max_size) {
      return std::nullopt;  // Error: seek beyond buffer size
    }
    byte_pos = new_pos;
    bit_pos = 0;  // Reset bit position when seeking to a new byte
    return byte_pos;
  }
};

// Specialization for file streams
template <>
class StreamHandler<FileStreamTag> {
public:
  static std::optional<std::size_t> read(std::ifstream& ifs, std::size_t& byte_pos, std::size_t& bit_pos, char* dest, std::size_t bits_to_read) {
    if (bit_pos != 0) {
      return std::nullopt;  // Error: Cannot start reading at a bit boundary in file streams
    }
    ifs.seekg(byte_pos);
    std::size_t bytes_to_read = (bits_to_read + CHAR_BIT - 1) / CHAR_BIT;  // Calculate full bytes to read
    if (!ifs.read(dest, bytes_to_read)) {
      return std::nullopt;  // Error: failed to read from file
    }
    byte_pos += bytes_to_read;
    return bits_to_read;
  }

  static std::optional<std::size_t> write(std::ofstream& ofs, std::size_t& byte_pos, std::size_t& bit_pos, const char* src, std::size_t bits_to_write) {
    if (bit_pos != 0) {
      return std::nullopt;  // Error: Cannot start writing at a bit boundary in file streams
    }
    ofs.seekp(byte_pos);
    std::size_t bytes_to_write = (bits_to_write + CHAR_BIT - 1) / CHAR_BIT;  // Calculate full bytes to write
    if (!ofs.write(src, bytes_to_write)) {
      return std::nullopt;  // Error: failed to write to file
    }
    byte_pos += bytes_to_write;
    return bits_to_write;
  }

  static std::optional<std::size_t> seek(std::size_t& byte_pos, std::size_t& bit_pos, std::size_t new_pos, std::size_t max_size) {
    byte_pos = new_pos;
    bit_pos = 0;  // Reset bit position when seeking to a new byte
    return byte_pos;
  }
};


template <typename Container, typename StreamType>
class MemStream {
public:
  explicit MemStream(Container& buffer)
      : buffer_(buffer), byte_pos_(0), bit_pos_(0) {}

  // Read a specific number of bits into the destination buffer
  template <std::size_t N>
  std::optional<std::size_t> read(char* dest, bits_to_read<N>) {
    return StreamHandler<StreamType>::read(buffer_, byte_pos_, bit_pos_, dest, N);
  }

  // Read a specific number of bytes into the destination buffer
  template <std::size_t N>
  std::optional<std::size_t> read(char* dest, bytes_to_read<N>) {
    return StreamHandler<StreamType>::read(buffer_, byte_pos_, bit_pos_, dest, N * CHAR_BIT);
  }

  // Write a specific number of bits from the source buffer
  template <std::size_t N>
  std::optional<std::size_t> write(const char* src, bits_to_read<N>) {
    return StreamHandler<StreamType>::write(buffer_, byte_pos_, bit_pos_, src, N);
  }

  // Write a specific number of bytes from the source buffer
  template <std::size_t N>
  std::optional<std::size_t> write(const char* src, bytes_to_read<N>) {
    return StreamHandler<StreamType>::write(buffer_, byte_pos_, bit_pos_, src, N * CHAR_BIT);
  }

  // Seek to a specific position in the buffer
  std::optional<std::size_t> seek(std::size_t pos) {
    return StreamHandler<StreamType>::seek(byte_pos_, bit_pos_, pos, buffer_.size());
  }

  // Get the current byte position in the buffer
  std::size_t tellg() const {
    return byte_pos_;
  }

  // Check if the end of the stream has been reached
  bool eof() const {
    return byte_pos_ >= buffer_.size();
  }

private:
  Container& buffer_;          // Reference to the buffer being used
  std::size_t byte_pos_;       // Current byte position in the buffer
  std::size_t bit_pos_;        // Current bit position in the buffer
};


static inline void test() {
  std::vector<char> buffer(10, 0); // Create a buffer with 10 bytes initialized to 0
  MemStream<std::vector<char>, MemoryStreamTag> memstream(buffer);
  char data[] = {0x01, 0x02, 0x03};
  memstream.write(data, 3_B); // Write 3 bytes
  memstream.seek(0); // Seek back to the beginning
  char read_buffer[3];
  memstream.read(read_buffer, 3_B); // Read 3 bytes
  for (char byte : read_buffer) {
    std::cout << std::hex << static_cast<int>(byte) << " ";
  }

  std::ifstream file("example.bin", std::ios::binary);
  MemStream<std::ifstream, FileStreamTag> file_stream(file);
  char read_buffer_1[5];
  file_stream.read(read_buffer_1, 5_B); // Read 5 bytes from file
  for (char byte : read_buffer_1) {
    std::cout << std::hex << static_cast<int>(byte) << " ";
  }
  file.close();

  std::vector<char> buffer_2(10, 0);
  MemStream<std::vector<char>, MemoryStreamTag> memstream_2(buffer);
  char data_2 = 0b10101010;
  memstream_2.write(&data_2, 8_b); // Write 8 bits
  memstream_2.seek(0);
  char read_data = 0;
  memstream_2.read(&read_data, 8_b); // Read 8 bits
  std::cout << std::bitset<8>(read_data) << std::endl; // Output: 10101010
}

#endif /*_MEMSTREAM_HPP_*/
