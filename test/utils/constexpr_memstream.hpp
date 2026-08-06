#ifndef _CONSTEXPR_MEMSTREAM_HPP_
#define _CONSTEXPR_MEMSTREAM_HPP_

#include <array>
#include "../../single_header/s2s.hpp"

using u8 = unsigned char;

template <std::size_t N>
class memstream: s2s::constexpr_stream {

private:
  std::array<u8, N>& buffer;
  std::size_t read_idx;
  std::size_t max_read_idx;
  std::size_t write_idx;
  bool is_bad;

public:
  constexpr memstream(std::array<u8, N>& buffer):
    buffer(buffer), read_idx{0}, max_read_idx{buffer.size() - 1}, write_idx{0}, is_bad{false} {}

  template <std::size_t dest_buffer_size>
  [[nodiscard]] constexpr auto read(std::array<char, dest_buffer_size>& dest, std::size_t size_to_read) -> memstream<N>& {
    std::size_t idx{0};
    while(idx < size_to_read) {
      if(read_idx > max_read_idx) {
        is_bad = true;
        return *this;
      }
      dest[idx++] = buffer[read_idx++];
    }
    return *this;
  }

  template <std::size_t src_buffer_size>
  [[nodiscard]] constexpr auto write(const std::array<char, src_buffer_size>& src, std::size_t size_to_write) -> memstream<N>& {
    std::size_t idx{0};
    while(idx < size_to_write) {
      if(write_idx >= buffer.size()) {
        is_bad = true;
        return *this;
      }
      buffer[write_idx++] = static_cast<u8>(src[idx++]);
    }
    return *this;
  }

  // Round-trip helper: rewind so a just-written buffer can be read back.
  constexpr auto rewind() -> void {
    read_idx = 0;
  }

  [[nodiscard]]
  explicit constexpr operator bool() const {
    return !is_bad;
  }
};

#endif /* _CONSTEXPR_MEMSTREAM_HPP_ */
