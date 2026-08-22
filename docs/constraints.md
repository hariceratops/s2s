# Constraints and validation

The `constraint` axis narrows which values a field may hold. It is the last
parameter of every descriptor that takes one, and it defaults to
`no_constraint`, so leaving it off means "any value the type can represent".

A constraint is checked in **both** directions:

- **reading** — after the bytes are decoded, before the value is stored;
- **writing** — before the field's first byte reaches the stream.

A violation is `validation_failure` either way, with `failed_at` naming the
field. That symmetry is the point: a schema that refuses to parse a bad value
also refuses to produce one.

A union takes two more, and they are separate things: one in the type tag of an
alternative, constraining that alternative's payload, and one on the `variance`
itself, constraining the resolved `std::variant`. See
[Optional and variant fields](schema/optional-and-variant.md).

## The constructs

| Constraint | Holds when | Value type |
|---|---|---|
| `no_constraint<T>{}` | always | any |
| `eq{v}` | `value == v` | equality-comparable, non-floating-point |
| `neq{v}` | `value != v` | equality-comparable, non-floating-point |
| `lt{v}` | `value < v` | comparable |
| `gt{v}` | `value > v` | comparable |
| `lte{v}` | `value <= v` | comparable |
| `gte{v}` | `value >= v` | integral or `fixed_string` |
| `any_of{a, b, ...}` | `value` is one of the listed values | all arguments the same type |

They are used as values, not types, and CTAD deduces the parameter:

```cpp
s2s::basic_field<"channels", u16, 2_B, s2s::any_of{u16{1}, u16{2}}>
```

Two type restrictions are worth knowing before a template error explains them.
`eq` and `neq` exclude floating-point types outright — equality on a float
decoded from a wire format is a trap, so the library refuses it rather than
supporting it badly. `gte` is narrower still, admitting only integrals and
`fixed_string`.

## Range constraints do not currently compile

`range`, `is_in_open_range` and `is_in_closed_range` are present in the headers
and are **not usable**. `range`'s constructor applies `static_assert` to its own
function parameters, which are not constant expressions, so constructing one is
a hard compile error — and both range constraints take `range` arguments.

```cpp
constexpr s2s::range<int> r{1, 5};   // error: 'value1' is not a constant expression
```

See `dev/issues/026-range-constraints-do-not-compile.md`. Until that is fixed, a
bounded field is best expressed with `gte`/`lte`, or with `any_of` for a small
set. These three will join the table above once they work.

## The magic descriptors are constraints

`magic_string`, `magic_number` and `magic_byte_array` are not a separate
validation mechanism. Each is an ordinary descriptor whose constraint axis is
already filled in with `eq`:

```cpp
magic_string<"magic", "S2S">
// is
field<"magic", fixed_string<3>, 4_B, eq{fixed_string<3>("S2S")}>
```

which is why they take an `expected` value where other descriptors take a
constraint, and why a wrong magic value reports `validation_failure` like any
other constraint breach rather than an error of its own.

<!-- docs: test/doc_examples/guide_constraints_example.cpp -->
```cpp
#include "s2s.hpp"

#include <array>
#include <fstream>

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

// The fmt chunk of a WAV file. Most of what makes a fmt chunk valid is a
// constraint on a single field, so most of it is declared rather than checked.
using wav_format =
  s2s::struct_field_list<
    s2s::magic_byte_array<"chunk_id", 4, std::array<u8, 4>{0x66, 0x6d, 0x74, 0x20}>,
    s2s::basic_field<"chunk_size", u32, 4_B, s2s::eq{u32{16}}>,
    s2s::basic_field<"audio_format", u16, 2_B, s2s::eq{u16{1}}>,
    s2s::basic_field<"channels", u16, 2_B, s2s::any_of{u16{1}, u16{2}}>,
    s2s::basic_field<"sample_rate", u32, 4_B, s2s::gte{u32{8000}}>,
    s2s::basic_field<"bits_per_sample", u16, 2_B, s2s::any_of{u16{8}, u16{16}, u16{24}}>
  >;

// The same layout with the constraint axis left off every field.
using wav_format_unchecked =
  s2s::struct_field_list<
    s2s::magic_byte_array<"chunk_id", 4, std::array<u8, 4>{0x66, 0x6d, 0x74, 0x20}>,
    s2s::basic_field<"chunk_size", u32, 4_B>,
    s2s::basic_field<"audio_format", u16, 2_B>,
    s2s::basic_field<"channels", u16, 2_B>,
    s2s::basic_field<"sample_rate", u32, 4_B>,
    s2s::basic_field<"bits_per_sample", u16, 2_B>
  >;

auto main() -> int {
  wav_format fmt{};
  fmt["chunk_id"_f] = std::array<u8, 4>{0x66, 0x6d, 0x74, 0x20};
  fmt["chunk_size"_f] = 16u;
  fmt["audio_format"_f] = u16{1};
  fmt["channels"_f] = u16{2};
  fmt["sample_rate"_f] = 44100u;
  fmt["bits_per_sample"_f] = u16{16};

  std::fstream file("wav_format.bin",
                    std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

  const auto accepted =
    s2s::stream_cast_be<wav_format>(file, fmt)
      .and_then([&file] {
        file.seekg(0);
        return s2s::struct_cast_be<wav_format>(file);
      })
      .transform([](const wav_format&) { return true; });

  if(!accepted.value_or(false))
    return 1;

  // Writing: the constraint is checked before the field's first byte is out.
  wav_format surround = fmt;
  surround["channels"_f] = u16{6};
  std::fstream discard("wav_format_bad.bin",
                       std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

  const auto refused = s2s::stream_cast_be<wav_format>(discard, surround);
  if(refused.has_value()
     || refused.error().failure_reason != s2s::error_reason::validation_failure
     || refused.error().failed_at != std::string_view{"channels"})
    return 1;

  // Reading: the same constraint rejects the same value coming off the wire.
  // The bytes have to exist first, so they are written through the unchecked
  // layout and parsed back through the checked one.
  wav_format_unchecked loose{};
  loose["chunk_id"_f] = std::array<u8, 4>{0x66, 0x6d, 0x74, 0x20};
  loose["chunk_size"_f] = 16u;
  loose["audio_format"_f] = u16{1};
  loose["channels"_f] = u16{6};
  loose["sample_rate"_f] = 44100u;
  loose["bits_per_sample"_f] = u16{16};

  std::fstream on_disk("wav_format_loose.bin",
                       std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

  const auto parsed =
    s2s::stream_cast_be<wav_format_unchecked>(on_disk, loose)
      .and_then([&on_disk] {
        on_disk.seekg(0);
        return s2s::struct_cast_be<wav_format>(on_disk);
      });

  if(parsed.has_value())
    return 1;

  return parsed.error().failure_reason == s2s::error_reason::validation_failure
      && parsed.error().failed_at == std::string_view{"channels"} ? 0 : 1;
}
```
