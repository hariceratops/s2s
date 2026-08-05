#include <array>
#include <expected>
#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"


using namespace s2s_literals;

using u32 = unsigned int;

using our_struct =
  s2s::struct_field_list<
    s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>
  >;


constexpr auto populated() -> our_struct {
  our_struct obj{};
  obj["a"_f] = 0xdeadbeef;
  obj["b"_f] = 0xcafed00d;
  return obj;
}

constexpr auto roundtrip_le() -> bool {
  std::array<u8, 8> buffer{};
  memstream<8> stream(buffer);
  if(!s2s::struct_write_le<our_struct>(stream, populated()))
    return false;
  stream.rewind();
  auto res = s2s::struct_cast_le<our_struct>(stream);
  return res && (*res)["a"_f] == 0xdeadbeef && (*res)["b"_f] == 0xcafed00d;
}

constexpr auto roundtrip_be() -> bool {
  std::array<u8, 8> buffer{};
  memstream<8> stream(buffer);
  if(!s2s::struct_write_be<our_struct>(stream, populated()))
    return false;
  stream.rewind();
  auto res = s2s::struct_cast_be<our_struct>(stream);
  return res && (*res)["a"_f] == 0xdeadbeef && (*res)["b"_f] == 0xcafed00d;
}

// A buffer one byte short of the schema: the write must report exhaustion
// rather than silently truncating.
constexpr auto write_into_undersized_buffer() -> s2s::cast_result {
  std::array<u8, 7> buffer{};
  memstream<7> stream(buffer);
  return s2s::struct_write_le<our_struct>(stream, populated());
}

static_assert(roundtrip_le(), "little-endian constexpr round-trip failed");
static_assert(roundtrip_be(), "big-endian constexpr round-trip failed");
static_assert(!write_into_undersized_buffer().has_value());
static_assert(
  write_into_undersized_buffer().error().failure_reason == s2s::error_reason::buffer_exhaustion);
static_assert(write_into_undersized_buffer().error().failed_at == "b");

auto main() -> int {
  return 0;
}
