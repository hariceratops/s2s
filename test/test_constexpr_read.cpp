#include <ios>
#include <iostream>
#include <expected>
#include <array>
#include "../include/s2s.hpp"


using u8 = unsigned char;


constexpr auto comptime_memcpy(char* dest, char* src, std::size_t size_to_copy) {
  std::size_t idx{0};
  while(idx < size_to_copy) {
    dest[idx] = src[idx];
    idx++;
  }
}

template <std::size_t N>
class memstream: s2s::constexpr_stream {

private:
  std::array<u8, N>& buffer;
  std::size_t read_idx;
  std::size_t max_read_idx;
  bool is_bad;

public:
  constexpr memstream(std::array<u8, N>& buffer): 
    buffer(buffer), read_idx{0}, max_read_idx{buffer.size() - 1}, is_bad{false} {}

  template <std::size_t dest_buffer_size>
  [[nodiscard]] constexpr auto read(std::array<char, dest_buffer_size>& dest, std::size_t size_to_read) -> memstream<N>& {
    std::size_t idx{0};
    while(idx < size_to_read) {
      // std::cout << "idx=" << idx << ' ' <<  "read_idx=" << read_idx << " " << is_bad << '\n';
      if(read_idx > max_read_idx) {
        // std::cout << "buffer_exhaust" << '\n';
        is_bad = true;
        return *this;
      }
      dest[idx++] = buffer[read_idx++];
    }
    return *this;
  }

  [[nodiscard]]
  explicit constexpr operator bool() const {
    return !is_bad;
  }
};


using u32 = unsigned int;
using our_struct =
  s2s::struct_field_list<
    s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>
  >;


constexpr auto parse_our_struct() -> std::expected<our_struct, s2s::cast_error>
{
  std::array<u8, 8> buffer{0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe, 0xca};
  memstream<8> stream(buffer);
  return s2s::struct_cast_le<our_struct>(stream);
}

constexpr auto failing_parse_our_struct() -> std::expected<our_struct, s2s::cast_error> 
{
  std::array<u8, 7> buffer{0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe};
  memstream<7> stream(buffer);
  return s2s::struct_cast_le<our_struct>(stream);
}

constexpr auto res = parse_our_struct();
static_assert(res);
constexpr auto fields = *res;
static_assert(fields["a"_f] == 0xdeadbeef);
static_assert(fields["b"_f] == 0xcafed00d);

constexpr auto fail_res = failing_parse_our_struct();
static_assert(not fail_res);
constexpr auto err = fail_res.error();
static_assert(err.failed_at == "b");
static_assert(err.failure_reason == s2s::error_reason::buffer_exhaustion);


void test_001() {
  auto res = failing_parse_our_struct();
  if(res) {
    auto fields = *res;
    std::cout << std::hex << fields["a"_f] << " " << fields["b"_f] << '\n';
  } else {
    std::cout << static_cast<int>(res.error().failure_reason) << " " << res.error().failed_at << '\n';
  }
}

void test_002() {
  auto res = parse_our_struct();
  if(res) {
    auto fields = *res;
    std::cout << std::hex << fields["a"_f] << " " << fields["b"_f] << '\n';
  } else {
    std::cout << static_cast<int>(res.error().failure_reason) << " " << res.error().failed_at << '\n';
  }
}

auto main(void) -> int {
  test_001();
  test_002();
  return 0;
}

