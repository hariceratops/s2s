# Computed values

Three different axes need a value worked out from other fields rather than
stated outright: a length the wire does not carry directly, a predicate deciding
whether an optional field is present, a test choosing a union alternative. They
share one shape — a callable, plus the names of the fields it reads.

**`with_fields<"a", "b">`** is that list of names. It carries no values; it says
which siblings to fetch and in what order, and they reach the callable as
arguments in exactly that order.

**`compute<callable, R, Fs>`** is the general form: apply `callable` to the
fields named by `Fs`, producing an `R`. The callable is a non-type template
argument, so it must be usable in a constant expression — a captureless lambda
or a function object declared where the schema can name it.

Everything else in this family is that template under a shorter name.

| Alias | Expands to | Reads as |
|---|---|---|
| `predicate<f, Fs>` | `compute<f, bool, Fs>` | a yes/no test over sibling fields |
| `eval_bool_from_fields<f, Fs>` | `compute<f, bool, Fs>` | the same thing, older spelling |
| `parse_if<f, Fs>` | `eval_bool_from_fields<f, Fs>` | is this optional field present? |
| `eval_size_from_fields<f, Fs>` | `compute<f, std::size_t, Fs>` | a count derived from siblings |

`compute` is documented here as a mechanism to be used directly, not as an
implementation detail behind those aliases. Deriving a `u32` from two sibling
fields is a legitimate thing to write, and the test suite already does it.
**The signature of `compute<callable, R, Fs>` should be treated as stable**:
documenting the general form commits the project to keeping it, not just the
aliases built on top.

One near relative is deliberately absent from that table. `size_from_fields`,
and its alias `len_from_fields`, live on the [size axis](size-axis.md) and take
the identical callable-plus-`with_fields` shape, but they are a separate
template rather than an alias of `compute`. The distinction rarely matters when
writing a schema and matters a great deal when reading a compiler error, which
is why it is called out here.

<!-- docs: test/single_header/guide_computed_example.cpp -->
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
  if(!file)
    return 1;

  if(const auto written = s2s::struct_write_be<image_tile>(file, tile); !written)
    return 1;

  file.seekg(0);
  const auto parsed = s2s::struct_cast_be<image_tile>(file);
  if(!parsed || (*parsed)["pixels"_f].size() != 8)
    return 1;

  // A tile claiming dimensions its pixel run cannot satisfy is rejected rather
  // than written as a stream that will not read back.
  image_tile inconsistent = tile;
  inconsistent["height"_f] = u16{3};
  std::fstream discard("image_tile_bad.bin",
                       std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
  const auto rejected = s2s::struct_write_be<image_tile>(discard, inconsistent);

  return !rejected
      && rejected.error().failure_reason == s2s::error_reason::found_contradicting_length
        ? 0 : 1;
}
```
