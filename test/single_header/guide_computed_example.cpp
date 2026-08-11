// Backs the computed-values example in docs/schema/computed-values.md. The region between
// docs-begin and docs-end is compared against that fenced block by the
// doc_examples_match test, so edit them together or the check fails.
// docs-begin
#include "s2s.hpp"

#include <fstream>
#include <vector>

using namespace s2s_literals;

using u16 = unsigned short;

// The callable is a non-type template argument, so it has to be usable in a
// constant expression and named somewhere the schema can reach.
constexpr auto pixel_count = [](auto width, auto height) { return width * height; };

// An image tile. Nothing on the wire states how many pixels follow — the count
// is the product of two fields that are each meaningful on their own.
using image_tile =
  s2s::struct_field_list<
    s2s::basic_field<"width", u16, s2s::field_size<s2s::fixed<2>>>,
    s2s::basic_field<"height", u16, s2s::field_size<s2s::fixed<2>>>,
    s2s::vec_field<
      "pixels",
      u16,
      s2s::field_size<s2s::len_from_fields<pixel_count, s2s::with_fields<"width", "height">>>
    >
  >;

auto main() -> int {
  image_tile tile{};
  // "width" and "height" stay assignable: multiplication cannot be run
  // backwards, so they are verified against pixels.size() rather than derived.
  tile["width"_f] = u16{4};
  tile["height"_f] = u16{2};
  tile["pixels"_f] = std::vector<u16>(8, 0x7fff);

  std::fstream file("image_tile.bin",
                    std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

  const auto round_tripped =
    s2s::stream_cast_be<image_tile>(file, tile)
      .and_then([&file] {
        file.seekg(0);
        return s2s::struct_cast_be<image_tile>(file);
      })
      .transform([](const image_tile& parsed) {
        return parsed["pixels"_f].size() == 8;
      });

  if(!round_tripped.value_or(false))
    return 1;

  // A tile claiming dimensions its pixel run cannot satisfy is rejected rather
  // than written as a stream that will not read back.
  image_tile inconsistent = tile;
  inconsistent["height"_f] = u16{3};
  std::fstream discard("image_tile_bad.bin",
                       std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

  const auto rejected = s2s::stream_cast_be<image_tile>(discard, inconsistent);
  if(rejected.has_value())
    return 1;

  return rejected.error().failure_reason == s2s::error_reason::found_contradicting_length
    ? 0 : 1;
}
// docs-end
