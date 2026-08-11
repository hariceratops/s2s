// Backs the constexpr example in UserGuide.md's Compile time section. The
// region between docs-begin and docs-end is compared against that fenced block
// by the doc_examples_match test, so edit them together or the check fails.
// docs-begin
#include "s2s.hpp"

#include <array>
#include <cstddef>

using namespace s2s_literals;

using u8 = unsigned char;
using u32 = unsigned int;

// A constexpr stream is identified by deriving from s2s::constexpr_stream, and
// works on std::array rather than char* so it is usable in constant evaluation.
template <std::size_t N>
class memstream: s2s::constexpr_stream {
  std::array<u8, N>& buffer;
  std::size_t read_idx{0};
  bool ok{true};

public:
  constexpr explicit memstream(std::array<u8, N>& b): buffer(b) {}

  template <std::size_t dest_size>
  [[nodiscard]] constexpr auto read(std::array<char, dest_size>& dest,
                                    std::size_t size_to_read) -> memstream& {
    for(std::size_t i = 0; i < size_to_read; ++i) {
      if(read_idx >= N) {
        ok = false;
        return *this;
      }
      dest[i] = static_cast<char>(buffer[read_idx++]);
    }
    return *this;
  }

  [[nodiscard]] explicit constexpr operator bool() const { return ok; }
};

// A fixed-layout partition entry. No allocating fields, so this schema is
// eligible for a compile-time parse.
using partition_entry =
  s2s::struct_field_list<
    s2s::basic_field<"start_lba", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"sector_count", u32, s2s::field_size<s2s::fixed<4>>>
  >;

constexpr auto parse_it() -> std::expected<partition_entry, s2s::cast_error> {
  std::array<u8, 8> buffer{0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xd0, 0x0d};
  memstream<8> stream(buffer);
  return s2s::struct_cast_be<partition_entry>(stream);
}

// The parse, the validation and the field lookups all happen in the compiler.
constexpr auto result = parse_it();
static_assert(result);
static_assert((*result)["start_lba"_f] == 0xdeadbeef);
static_assert((*result)["sector_count"_f] == 0xcafed00d);

auto main() -> int {
  return 0;
}
// docs-end
