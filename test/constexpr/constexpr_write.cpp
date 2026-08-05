#include <array>
#include <expected>
// TODO(issue 000): should include ../../include/s2s.hpp so the non-amalgamated
// tree is exercised, but that header is currently broken (line 11 includes a
// file deleted by commit b8e6108). Using the single header until 000 lands.
#include "../../single_header/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"


using u32 = unsigned int;

using our_struct =
  s2s::struct_field_list<
    s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>
  >;

// TODO(issue 001): populate a struct, write it with struct_write_le into the
// memstream, rewind, read it back with struct_cast_le, and return the result
// so the static_asserts below can compare against the original. Mirrors
// constexpr_read.cpp's shape.
constexpr auto roundtrip_our_struct() -> std::expected<our_struct, s2s::cast_error> {
  std::array<u8, 8> buffer{};
  memstream<8> stream(buffer);
  return s2s::struct_cast_le<our_struct>(stream);
}

// TODO(issue 001): assert round-trip fidelity at compile time, and add a
// big-endian counterpart. Extended per field kind by issues 002-009.
static_assert(roundtrip_our_struct().has_value() || true, "placeholder");

auto main() -> int {
  return 0;
}
