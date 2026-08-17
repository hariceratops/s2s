# Reading

```cpp
template <struct_field_list_like T, stream_like S>
auto struct_cast_le(S& stream) -> std::expected<T, cast_error>;

template <struct_field_list_like T, stream_like S>
auto struct_cast_be(S& stream) -> std::expected<T, cast_error>;
```

The schema is the template argument, the stream is the runtime one. The `le` and
`be` suffix fixes the byte order of every integral member of the schema — it is
not a per-field setting. Success yields the populated `struct_field_list`;
failure yields a `cast_error`.

The struct is read strictly left to right, one field at a time, in declaration
order and at every nesting level. That ordering is not an implementation detail
to be ignored: it is why a `len_from_field` must name a field declared *before*
the one it sizes, and why a `parse_if` predicate can only read fields already
parsed.

Which field kinds exist, and how many bytes each consumes, is part of the schema
language rather than of this direction — see the descriptor table in
[Schema](schema/index.md).

## A worked example

<!-- docs: test/doc_examples/guide_reading_example.cpp -->
```cpp
#include "s2s.hpp"

#include <array>
#include <fstream>

using namespace s2s_literals;

using u8 = unsigned char;
using u32 = unsigned int;

// The 14-byte header every BMP file starts with. BMP is a little-endian
// format, so it is read with struct_cast_le.
using bmp_header =
  s2s::struct_field_list<
    s2s::magic_byte_array<"signature", 2, std::array<u8, 2>{0x42, 0x4d}>,
    s2s::basic_field<"file_size", u32, 4_B>,
    s2s::basic_field<"reserved", u32, 4_B>,
    s2s::basic_field<"pixel_offset", u32, 4_B>
  >;

// 'B' 'M', then file_size, reserved and pixel_offset, each little-endian.
constexpr unsigned char header_bytes[] = {
  0x42, 0x4d,
  0x36, 0x00, 0x0c, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x36, 0x00, 0x00, 0x00
};

auto write_sample(const char* path, std::size_t count) -> bool {
  std::ofstream out(path, std::ios::out | std::ios::binary | std::ios::trunc);
  out.write(reinterpret_cast<const char*>(header_bytes),
            static_cast<std::streamsize>(count));
  return static_cast<bool>(out);
}

auto main() -> int {
  if(!write_sample("sample.bmp", sizeof(header_bytes)))
    return 1;

  std::ifstream file("sample.bmp", std::ios::in | std::ios::binary);

  const auto parsed =
    s2s::struct_cast_le<bmp_header>(file)
      .transform([](const bmp_header& header) {
        return header["file_size"_f] == 0x000c0036 && header["pixel_offset"_f] == 54;
      });

  if(!parsed.value_or(false))
    return 1;

  // A file that stops short reports which field ran out, not merely that one did.
  if(!write_sample("truncated.bmp", 8))
    return 1;

  std::ifstream short_file("truncated.bmp", std::ios::in | std::ios::binary);
  const auto failed = s2s::struct_cast_le<bmp_header>(short_file);
  if(failed.has_value())
    return 1;

  return failed.error().failure_reason == s2s::error_reason::buffer_exhaustion
      && failed.error().failed_at == std::string_view{"reserved"} ? 0 : 1;
}
```

## What a failed read leaves behind

`struct_cast` returns `std::expected`, so a failure gives back a `cast_error`
and no struct — there is no partially populated result to inspect. Whatever had
been parsed up to that point is discarded with the `expected`.

The stream is a different matter. The library does not own it, and on failure
its read position is wherever the failing field stopped, not rewound to where
the cast started. Code that intends to retry, or to try a second schema against
the same bytes, must record the position beforehand. Opening a fresh stream over
the same buffer is usually simpler.

## Read errors

| Cause | `failure_reason` | `failed_at` |
|---|---|---|
| the stream ran out mid-field | `buffer_exhaustion` | the field being read |
| a constraint rejected the decoded value | `validation_failure` | that field |
| a magic value did not match | `validation_failure` | the magic field |
| a constraint on a union alternative rejected its payload | `validation_failure` | the union field |
| no `match_case` matched and no `branch` predicate held | `type_deduction_failure` | the union field |
| a length exceeded the field's allocation ceiling | `excessive_length` | the container field |

Note the first and last rows are not the same failure. `buffer_exhaustion`
means a read began and the stream ran dry; `excessive_length` means the read
never began, because the length could not be allocated. A truncated file gives
the first, a corrupt length gives the second.

`found_contradicting_length`, the fourth `error_reason`, cannot arise on a read.
It reports two parts of a struct implying different lengths for the same data,
which is only detectable when writing — reading takes the length off the wire
and has nothing to disagree with it. See [Writing](writing.md).

For a failure inside a nested record, `failed_at` names the outermost record
field rather than the inner one. A validation failure two levels down inside
`struct_field<"header", ...>` reports `"header"`, so the name returned is always
one that appears in the schema handed to `struct_cast`.

## Allocation limits

A length-prefixed field's size comes off the wire, so a corrupt or hostile
stream can claim a length far larger than the data behind it. Reading such a
field naively means allocating for the claim before discovering it was a lie —
a four-byte length can ask for four gigabytes.

**Every variable-sized field is bounded, whether or not the schema says so.**
The default ceiling is 16 MiB of allocated memory per field, and a length
exceeding it fails with `excessive_length` *before* anything is allocated. This
is the one read error that fires ahead of the read rather than during it, which
is the whole point of it: `buffer_exhaustion` tells you the stream ran dry,
`excessive_length` tells you it never had a chance.

Declare a different ceiling per field with `max_bytes`:

```cpp
s2s::vec_field<"payload", u8, s2s::len_from_field<"n">, s2s::max_bytes<4096>>
```

It is a trailing option like any other, so it composes with a size and a
constraint in any order. The bound is denominated in **bytes of allocated
memory** — `count * sizeof(element)` — which for `vector_of_records` means the
records' in-memory footprint rather than the bytes they occupy on the wire.
`max_bytes` is inclusive: exactly the limit is accepted.

It applies only where a length off the wire drives an allocation, which is
`vec_field`, `str_field` and `vector_of_records`. A fixed-size field, a
`fixed_array_field` or an `array_of_records` has an extent fixed at compile
time that no stream can influence, and declaring `max_bytes` on one is a
compile error rather than a no-op — silently ignoring it would let you believe
you had bounded something you had not.

### Raising the default

`S2S_DEFAULT_MAX_BYTES` sets the default for every field that declares nothing.
Define it before including the header:

```cpp
#define S2S_DEFAULT_MAX_BYTES (256u * 1024u * 1024u)
#include "s2s.hpp"
```

Setting it to `SIZE_MAX` disables the defaults entirely. **It cannot disable a
`max_bytes` you declared** — a default is the library's guess and yours to
overrule, but a declared limit is your own intent, and no build setting
discards it.

One gap worth knowing: a `vec` alternative inside a `variance` cannot declare
its own ceiling. A type tag's option pack admits a size and a constraint, but
not `max_bytes`. Such a field still gets the default, so it is bounded; it
simply cannot be given a different bound short of moving the global default.
