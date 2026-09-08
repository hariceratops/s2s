# The byte-order axis

Most formats fix their byte order once and for all: BMP is little-endian, and
every field in it is read little-endian. Those schemas name the order at the
call — `struct_cast_le` or `struct_cast_be` — and nothing about the file changes
it.

Some formats decide per file. TIFF opens with `II` or `MM`, pcap with a magic
number written one way round or the other, ELF with a byte at offset 5. For
those, the order is data: it is in the file, and the schema has to say which
field carries it and what its values mean. That is what this axis declares, and
a schema that uses it is read with `struct_cast` and written with `stream_cast`,
neither of which takes an order at all — the file supplies it.

## Declaring an announcing record

```cpp
template <fixed_string id, field_list_like T, order_deduction_like guide,
          constraint_option_like<T> auto... opts>
using announces_byte_order = /* a record field, marked */;
```

`announces_byte_order` wraps a record — an ordinary `struct_field_list` — and
attaches a guide saying how that record's parsed fields resolve to a
`std::endian`. The record is read like any other nested record; the marking only
adds what happens the moment its last field leaves the stream.

The guide is spelled the way [`variance`'s type
deduction](optional-and-variant.md#variance-type-deduction) is spelled, with
`order_from` in place of `type` and a byte order in place of an alternative:

```cpp
s2s::order_from<s2s::match_field<"marker">,
  s2s::order_switch<
    s2s::order_case<std::array<u8, 2>{'I', 'I'}, std::endian::little>,
    s2s::order_case<std::array<u8, 2>{'M', 'M'}, std::endian::big>>>
```

Field names inside a guide resolve in the **announcing record's own** field
list, not in the record that declares the announcement. Naming a field that
record does not have is a compile error saying so.

The schema speaks `std::endian` rather than anything relative to the machine:
`II` means little-endian on every host, and what that costs on the machine
running the parse is the library's problem.

## A worked example

<!-- docs: test/doc_examples/guide_byte_order_example.cpp -->
```cpp
#include "s2s.hpp"

#include <array>
#include <bit>
#include <fstream>

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

// A TIFF file opens with two bytes naming the byte order the rest of it uses:
// "II" for little-endian, "MM" for big. Everything after them — starting with
// the 42 that confirms the format — is read in whichever order they name.
using byte_order_marker =
  s2s::struct_field_list<
    s2s::fixed_array_field<"marker", u8, 2>
  >;

using tiff_header =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", byte_order_marker,
      s2s::order_from<s2s::match_field<"marker">,
        s2s::order_switch<
          s2s::order_case<std::array<u8, 2>{'I', 'I'}, std::endian::little>,
          s2s::order_case<std::array<u8, 2>{'M', 'M'}, std::endian::big>>>>,
    s2s::magic_number<"magic", u16, 2_B, 42>,
    s2s::basic_field<"first_ifd_offset", u32, 4_B>
  >;

// The same header in both orders. Only the first two bytes say so; the four
// after them are the same number written two ways.
constexpr unsigned char little_endian_tiff[] = {
  'I', 'I', 0x2a, 0x00, 0x08, 0x00, 0x00, 0x00
};

constexpr unsigned char big_endian_tiff[] = {
  'M', 'M', 0x00, 0x2a, 0x00, 0x00, 0x00, 0x08
};

auto write_sample(const char* path, const unsigned char (&bytes)[8]) -> bool {
  std::ofstream out(path, std::ios::out | std::ios::binary | std::ios::trunc);
  out.write(reinterpret_cast<const char*>(bytes), 8);
  return static_cast<bool>(out);
}

auto first_ifd_offset(const char* path) -> std::expected<u32, s2s::cast_error> {
  std::ifstream file(path, std::ios::in | std::ios::binary);
  return s2s::struct_cast<tiff_header>(file)
    .transform([](const tiff_header& header) { return header["first_ifd_offset"_f]; });
}

auto main() -> int {
  if(!write_sample("little.tiff", little_endian_tiff))
    return 1;
  if(!write_sample("big.tiff", big_endian_tiff))
    return 1;

  // One schema, one call, no byte order named anywhere: the files decide, and
  // both yield 8.
  if(first_ifd_offset("little.tiff").value_or(0) != 8)
    return 1;
  if(first_ifd_offset("big.tiff").value_or(0) != 8)
    return 1;

  // On the way out the marker held in the struct decides the order, so setting
  // it is how a caller chooses what the file will look like.
  tiff_header header{};
  header["byte_order"_f]["marker"_f] = std::array<u8, 2>{'M', 'M'};
  header["first_ifd_offset"_f] = 8;

  std::ofstream out("written.tiff", std::ios::out | std::ios::binary | std::ios::trunc);
  if(!s2s::stream_cast<tiff_header>(out, header))
    return 1;
  out.close();

  // A file whose marker is neither II nor MM is not a TIFF, and is rejected the
  // way a bad magic number is: validation_failure, at the announcing field.
  constexpr unsigned char not_a_tiff[] = {
    'X', 'X', 0x2a, 0x00, 0x08, 0x00, 0x00, 0x00
  };
  if(!write_sample("other.bin", not_a_tiff))
    return 1;

  const auto rejected = first_ifd_offset("other.bin");
  if(rejected.has_value())
    return 1;

  return rejected.error().failure_reason == s2s::error_reason::validation_failure
      && rejected.error().failed_at == std::string_view{"byte_order"} ? 0 : 1;
}
```

## The three deduction forms

A guide takes the same three inputs the type-deduction axis takes.

**`match_field`** switches on one field of the announcing record. It is the form
above, and it covers a marker that is a single field.

**A callable** switches on a value computed from several of them. This is the
form that justifies marking the *record* rather than one field: a UTF-16 byte
order mark is `0xff 0xfe` or `0xfe 0xff`, the same two bytes the other way
round, so neither byte decides anything alone.

```cpp
constexpr auto bom_order = [](u8 first, u8 second) -> unsigned {
  if(first == 0xff && second == 0xfe) return 0u;
  if(first == 0xfe && second == 0xff) return 1u;
  return 2u;
};

using text_file =
  s2s::struct_field_list<
    s2s::announces_byte_order<"bom", byte_order_mark,
      s2s::order_from<s2s::compute<bom_order, unsigned, "first", "second">,
        s2s::order_switch<
          s2s::order_case<0u, std::endian::little>,
          s2s::order_case<1u, std::endian::big>>>>,
    s2s::fixed_array_field<"units", u16, 2>
  >;
```

**An if-else ladder** takes a predicate per branch, for conditions a single
match cannot express.

```cpp
s2s::order_from<
  s2s::order_if_else<
    s2s::order_branch<s2s::predicate<is_little_endian, "first", "second">, std::endian::little>,
    s2s::order_branch<s2s::predicate<is_big_endian, "first", "second">, std::endian::big>>>
```

A marker matching no case, and a ladder falling off its end, are the same
outcome: the file is not this format. Both are reported as a
`validation_failure` at the announcing field's id, the same way a magic number
that does not match is — see [Errors](../errors.md).

## Where the resolved order takes effect

**Resolution is a point in the byte stream, not a place in the schema tree.**
The order takes effect the instant the announcing record's last field is read,
and governs everything read after that point. It is resolved once and never
re-read.

That phrasing is load-bearing when the announcing record is nested. Given

```cpp
using container =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", marker, guide>,
    s2s::basic_field<"entry_count", u16, 2_B>
  >;

using file =
  s2s::struct_field_list<
    s2s::struct_field<"ifd", container>,
    s2s::basic_field<"next_ifd", u32, 4_B>
  >;
```

`entry_count` is read after the announcement and so takes the resolved order,
even though it sits *inside* the record that contains the announcement. So does
`next_ifd`, after the container closes. The announcing record may sit at any
depth; nothing about the rule changes with it.

## What an announcing record may contain

The announcing record is parsed before its own deduction runs, so its fields are
read before there is an order to read them in. Every field in it must therefore
read the same either way, and the compiler enforces it: a marker spelled as
bytes, as a string, or as single-byte integers is accepted, and a multi-byte
integer is rejected.

```cpp
// Accepted: two bytes have no byte order to get wrong.
s2s::fixed_array_field<"marker", u8, 2>
s2s::magic_byte_array<"marker", 2, std::array<u8, 2>{'I', 'I'}>
s2s::fixed_string_field<"tag", 3>

// Rejected: a u16 read before the order is known.
s2s::basic_field<"version", u16, 2_B>
```

The check descends into nested records inside the announcing record, and it
stops at the announcing record's boundary. Fields *after* the announcement carry
no restriction — that is the whole point of them, and a schema whose every later
field is order-dependent is exactly what this axis is for.

!!! warning "The check is narrower than the rule"

    The rule is that every field read before the announcement completes must be
    order-agnostic. What the library checks is the announcing record's own
    fields. A field placed as an **earlier sibling** of the announcing record is
    read before the order is known and is **not** diagnosed; it is read at an
    unspecified order.

    Formats this axis targets do not hit it — TIFF's marker is at byte 0, pcap's
    magic is at byte 0, and ELF's `EI_DATA` is preceded only by `\x7fELF` and
    two single bytes — but a schema that puts an order-dependent field ahead of
    its announcement is wrong and will not be told so. Widening the check to
    every field read before the announcement is additive and can land later.

## On write, the marker decides

The write direction mirrors the read direction exactly: the order takes effect
the instant the announcing record is written, at any depth, by the same rule.
What it resolves from is the marker already sitting in the struct — from a prior
parse, or set by the caller.

```cpp
tiff_header header{};
header["byte_order"_f]["marker"_f] = std::array<u8, 2>{'M', 'M'};
```

The marker is ordinary struct data and keeps its setter. It is not a field
[frozen by its constraint](../writing.md#fields-frozen-by-a-constraint-are-read-only-too):
assigning it is how a caller chooses the output order, and there is no separate
requested order to check it against.

Constraining a marker with `eq` — writing it as a `magic_byte_array`, say —
composes with that feature rather than conflicting with it. The marker loses its
setter and the schema can then only ever read and write the one order that value
decodes to, which is a fixed-order schema written the long way round. It is
legitimate, and it works without either feature knowing about the other.

## Limitations

Both are current limitations rather than design principles, and lifting either
is additive.

- **A schema may declare at most one announcing record.** A second one taking
  over from the first follows from the same stream-position rule at no extra
  cost. It is left undecided only until a real format settles whether the second
  overrides the first, and whether nesting is restricted. Two announcements
  anywhere in a schema is a compile error.
- **A resolved order always governs the rest of the file.** There is no way to
  scope an announcement to its own subtree, which is what an embedded format
  carrying its own marker would need — Exif MakerNotes is the standing example.
  Reading such a region as a separate cast is the workaround.
