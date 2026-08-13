# Computed values

Three different axes need a value worked out from other fields rather than
stated outright: a length the wire does not carry directly, a predicate deciding
whether an optional field is present, a test choosing a union alternative. They
share one shape — a callable, plus the names of the fields it reads.

The names are written as a bare list of ids — `"a", "b"` — trailing the
callable. There is no wrapper around them; the order is the order they reach
the callable as arguments.

**`compute<callable, R, "a", "b">`** is the general form: apply `callable` to
the fields named, producing an `R`. The callable is a non-type template
argument, so it must be usable in a constant expression — a captureless lambda
or a function object declared where the schema can name it.

Everything else in this family is that template under a shorter name.

| Alias | Expands to | Reads as |
|---|---|---|
| `predicate<f, ids...>` | `compute<f, bool, ids...>` | a yes/no test over sibling fields |
| `eval_bool_from_fields<f, ids...>` | `compute<f, bool, ids...>` | the same thing, older spelling |
| `parse_if<f, ids...>` | `eval_bool_from_fields<f, ids...>` | is this optional field present? |
| `eval_size_from_fields<f, ids...>` | `compute<f, std::size_t, ids...>` | a count derived from siblings |

`compute` is documented here as a mechanism to be used directly, not as an
implementation detail behind those aliases. Deriving a `u32` from two sibling
fields is a legitimate thing to write, and the test suite already does it.
**The signature of `compute<callable, R, ids...>` should be treated as stable**:
documenting the general form commits the project to keeping it, not just the
aliases built on top.

One near relative is deliberately absent from that table. `size_from_fields`,
and its alias `len_from_fields`, live on the [size axis](size-axis.md) and take
the identical callable-plus-`with_fields` shape, but they are a separate
template rather than an alias of `compute`. The distinction rarely matters when
writing a schema and matters a great deal when reading a compiler error, which
is why it is called out here.

<!-- docs: test/doc_examples/guide_computed_example.cpp -->
```cpp
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
    s2s::basic_field<"width", u16, 2_B>,
    s2s::basic_field<"height", u16, 2_B>,
    s2s::vec_field<
      "pixels",
      u16,
      s2s::len_from_fields<pixel_count, "width", "height">
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
```
