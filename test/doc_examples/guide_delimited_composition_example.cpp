// Backs the discovered-length-feeds-a-computed-one example in
// docs/schema/computed-values.md. The region between docs-begin and docs-end
// is compared against that fenced block by the doc_examples_match test, so
// edit them together or the check fails.
// docs-begin
#include "s2s.hpp"

#include <fstream>
#include <string>

using namespace s2s_literals;

using u8 = unsigned char;
using u32 = unsigned int;

// PNG's tEXt chunk: a NUL-terminated keyword, then text running to the end of
// the chunk. The text is explicitly not terminated — the chunk's declared
// length is what locates its end, so its size is the chunk length minus the
// keyword's own discovered length minus the one delimiter byte.
constexpr auto remaining = [](auto length, const std::string& keyword) -> std::size_t {
  return length - keyword.size() - 1;
};

using text_chunk =
  s2s::struct_field_list<
    s2s::basic_field<"length", u32, 4_B>,
    s2s::str_field<"keyword", s2s::until<u8{0}>, s2s::max_bytes<79>>,
    s2s::str_field<"text", s2s::size_from_fields<remaining, "length", "keyword">>
  >;

// length, "Author\0", "Claude Code" — 6 + 1 + 11 = 18.
constexpr unsigned char chunk_bytes[] = {
  0x00, 0x00, 0x00, 0x12,
  'A', 'u', 't', 'h', 'o', 'r', 0x00,
  'C', 'l', 'a', 'u', 'd', 'e', ' ', 'C', 'o', 'd', 'e'
};

auto main() -> int {
  {
    std::ofstream out("text_chunk.bin", std::ios::out | std::ios::binary | std::ios::trunc);
    out.write(reinterpret_cast<const char*>(chunk_bytes), sizeof(chunk_bytes));
  }

  std::ifstream file("text_chunk.bin", std::ios::in | std::ios::binary);

  const auto parsed =
    s2s::struct_cast_be<text_chunk>(file)
      .transform([](const text_chunk& chunk) {
        return chunk["keyword"_f] == "Author" && chunk["text"_f] == "Claude Code";
      });

  return parsed.value_or(false) ? 0 : 1;
}
// docs-end
