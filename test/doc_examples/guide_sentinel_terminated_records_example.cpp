// Backs the until_field_equals<> example in docs/schema/size-axis.md. The
// region between docs-begin and docs-end is compared against that fenced block
// by the doc_examples_match test, so edit them together or the check fails.
// docs-begin
#include "s2s.hpp"

#include <fstream>

using namespace s2s_literals;

using u8 = unsigned char;

// A GIF image's data is a run of sub-blocks, each a length byte and that many
// bytes of payload, ended by a sub-block whose length is zero. Nothing on the
// wire counts the sub-blocks; the zero-length one is the only thing that ends
// the run. The terminator is read and discarded, so "blocks" holds only the
// real sub-blocks.
using sub_block =
  s2s::struct_field_list<
    s2s::basic_field<"size", u8, 1_B>,
    s2s::vec_field<"data", u8, s2s::len_from_field<"size">>
  >;

using image_data =
  s2s::struct_field_list<
    s2s::basic_field<"lzw_min_code_size", u8, 1_B>,
    s2s::vector_of_records<"blocks", sub_block, s2s::until_field_equals<"size", u8{0}>>
  >;

auto main() -> int {
  {
    std::ofstream out("image_data.bin", std::ios::out | std::ios::binary | std::ios::trunc);
    out.write("\x08\x02" "ab" "\x01" "c" "\x00", 7);
  }

  std::ifstream file("image_data.bin", std::ios::in | std::ios::binary);

  const auto parsed =
    s2s::struct_cast_le<image_data>(file)
      .transform([](const image_data& image) {
        return image["lzw_min_code_size"_f] == 8
            && image["blocks"_f].size() == 2
            && image["blocks"_f][0]["data"_f].size() == 2
            && image["blocks"_f][1]["data"_f][0] == 'c';
      });

  return parsed.value_or(false) ? 0 : 1;
}
// docs-end
