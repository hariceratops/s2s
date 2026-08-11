# Schema

A schema is a type. It is declared once as a `struct_field_list` of field
descriptors, and that single declaration drives both directions: `struct_cast`
reads a stream into it, `struct_write` writes it back out.

Nothing in this section belongs to one direction. Field descriptors are the
schema language, not a read-side feature — where the two directions genuinely
differ is in what they *do* with a declaration, and that is covered in
[Reading](../reading.md) and [Writing](../writing.md).

## The four axes

Every field declares up to four things.

| Axis | What it answers | Required |
|---|---|---|
| `id` | what the field is called | always |
| `type` | which C++ type holds the value | always |
| `size` | how many bytes it occupies on the wire | unless the type implies it |
| `constraint` | which values are legal | no — defaults to `no_constraint` |

<!-- docs: test/single_header/guide_schema_example.cpp -->
```cpp
#include "s2s.hpp"

#include <array>
#include <fstream>
#include <vector>

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

// A telemetry frame: a two-byte marker, the device that sent it, a protocol
// revision, and a run of samples whose count is carried on the wire.
using telemetry_frame =
  s2s::struct_field_list<
    s2s::magic_byte_array<"marker", 2, std::array<u8, 2>{0xab, 0xcd}>,
    s2s::basic_field<"device_id", u16, s2s::field_size<s2s::fixed<2>>>,
    s2s::basic_field<"revision", u8, s2s::field_size<s2s::fixed<1>>, s2s::any_of{u8{1}, u8{2}}>,
    s2s::basic_field<"sample_count", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::vec_field<"samples", u16, s2s::field_size<s2s::len_from_field<"sample_count">>>
  >;

auto main() -> int {
  telemetry_frame frame{};
  frame["marker"_f] = std::array<u8, 2>{0xab, 0xcd};
  frame["device_id"_f] = u16{0x2a};
  frame["revision"_f] = u8{2};
  frame["samples"_f] = std::vector<u16>{300, 301, 299, 302};
  // "sample_count" is not assigned: it is the size axis of "samples" resolved.

  std::fstream file("telemetry_frame.bin",
                    std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
  if(!file)
    return 1;

  if(const auto written = s2s::struct_write_be<telemetry_frame>(file, frame); !written)
    return 1;

  // A file stream shares one position between reads and writes, so rewind
  // before parsing the bytes just emitted.
  file.seekg(0);

  const auto parsed = s2s::struct_cast_be<telemetry_frame>(file);
  if(!parsed)
    return 1;

  return (*parsed)["sample_count"_f] == 4
      && (*parsed)["device_id"_f] == 0x2a
      && (*parsed)["samples"_f][2] == 299 ? 0 : 1;
}
```

**`id`** is a string literal, and it is how the field is reached:
`frame["revision"_f]`. Ids are unique within a `struct_field_list` — a duplicate
is a compile error, and so is naming an id the schema does not contain, because
the lookup is a concept check rather than a runtime search.

**`type`** is an ordinary C++ type. Which types are admissible depends on the
descriptor: `basic_field` takes integrals, the record descriptors take another
`struct_field_list`, and the string and vector descriptors fix the type
themselves.

**`size`** says how many bytes the field occupies on the wire, which is not the
same question as how large its C++ type is. A `u32` field can be declared to
occupy two bytes. The axis has six forms and enough depth to warrant
[its own page](size-axis.md).

**`constraint`** narrows which values are legal. It is checked in both
directions — while reading, before the value is stored; while writing, before
the field's first byte is emitted — and a violation is a `validation_failure`
either way. See [Constraints and validation](../constraints.md).

## The descriptors

Each descriptor is an alias that fixes some axes and leaves the rest open.
`constraint` is the last parameter of every descriptor that accepts one, and
defaults to `no_constraint`.

The last column is what a reader most often wants: how many bytes the field
consumes off the wire, and where that count comes from.

| Descriptor | Wire concept | Type it holds | Bytes consumed |
|---|---|---|---|
| `basic_field<id, T, size, c>` | trivial | integral `T` | its `fixed<N>`, which must fit `sizeof(T)` |
| `fixed_array_field<id, T, N, c>` | array of trivials | `std::array<T, N>` | `N * sizeof(T)` |
| `c_arr_field<id, T, N, c>` | array of trivials | `T[N]` | `N * sizeof(T)` |
| `array_of_records<id, T, N, c>` | array of records | `std::array<T, N>`, `T` a schema | sum of `N` nested schemas |
| `fixed_string_field<id, N, c>` | const-sized string | `fixed_string<N>` | `N + 1`, terminator included |
| `c_str_field<id, N, c>` | const-sized string | `char[N + 1]` | `N + 1`, terminator included |
| `str_field<id, size, c>` | length-prefixed string | `std::string` | byte count from the size axis |
| `vec_field<id, T, size, c>` | length-prefixed vector of trivials | `std::vector<T>` | element count from the size axis |
| `vector_of_records<id, T, size, c>` | length-prefixed vector of records | `std::vector<T>`, `T` a schema | element count from the size axis |
| `struct_field<id, T>` | nested record | `T`, a schema | whatever the nested schema occupies |
| `magic_byte_array<id, N, expected>` | magic bytes | `std::array<unsigned char, N>` | `N`, and the value is checked |
| `magic_string<id, expected>` | magic string | `fixed_string<expected.size()>` | `size() + 1`, and the value is checked |
| `magic_number<id, T, size, expected>` | magic number | integral `T` | its declared size, and the value is checked |
| `maybe<field, present_only_if>` | optional | whatever it wraps | zero, or the wrapped field's |
| `variance<id, deducer>` | union | `std::variant` of the alternatives | the selected alternative's |

Four things in that table are easy to misread.

The `+ 1` on `fixed_string_field`, `c_str_field` and `magic_string` is a
terminator byte: a `fixed_string<3>` such as `"PKT"` occupies four bytes on the
wire, not three. A format whose magic is exactly `N` bytes wants
`magic_byte_array`, which has no terminator.

`size_dont_care`, used by the record descriptors, is not a size of zero. It
marks a field whose width is whatever its own nested schema turns out to
occupy, which is why it is never written by hand.

The magic descriptors are not a separate mechanism. Each is an ordinary
descriptor with an `eq` constraint already applied, which is why they take an
`expected` value where the others take a constraint.

Only two entries consume a variable number of bytes from one read to the next
rather than from one schema to the next: `maybe` consumes nothing when its
predicate is false, and `variance` consumes whatever the selected alternative
needs. Every other width is either fixed at compile time or read from an
earlier field.

`maybe` and `variance` are combinators rather than field kinds — they wrap or
select among the descriptors above, and are covered in
[Optional and variant fields](optional-and-variant.md).

## `fixed_string`

`fixed_string<N>` is part of the supported surface, not an internal detail. It
is unavoidable: `magic_string` and `fixed_string_field` use it as their field
type, so assigning to one means constructing one.

```cpp
header["magic"_f] = s2s::fixed_string<3>("PKT");
```

`N` is the length in characters, excluding the terminator, so the argument is a
four-character string literal. It offers `data()`, `size()`, `to_sv()`, and
comparison against other `fixed_string`s of any length.

## What this guide does not cover

`include/lib/` also holds `static_vector`, `static_map`, and a set of
metaprogramming helpers. These are implementation machinery for keeping compiler
diagnostics readable, they have no user-facing uses, and they are deliberately
undocumented. Treat them as internal and subject to change; `fixed_string` is
the exception, for the reason above.
