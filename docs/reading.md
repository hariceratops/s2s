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
| no `match_case` matched and no `branch` predicate held | `type_deduction_failure` | the union field |

`found_contradicting_length`, the fourth `error_reason`, cannot arise on a read.
It reports two parts of a struct implying different lengths for the same data,
which is only detectable when writing — reading takes the length off the wire
and has nothing to disagree with it. See [Writing](writing.md).

For a failure inside a nested record, `failed_at` names the outermost record
field rather than the inner one. A validation failure two levels down inside
`struct_field<"header", ...>` reports `"header"`, so the name returned is always
one that appears in the schema handed to `struct_cast`.
