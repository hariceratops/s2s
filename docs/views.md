# Views

A view reads the fields of a record straight out of a span of bytes the caller
already holds, without first parsing the whole record into a `struct_field_list`.
It takes the same schema as [reading](reading.md) does, and `v["name"_f]` has
the same spelling, but only the fields that are asked for are decoded.

## What a view is, and what it is not

**A view is not zero-copy.** Every field read is decoded from the bytes into a
local and returned **by value**, which is a copy. A field in the opposite byte
order has to be swapped, so it can never be handed out as a reference into the
buffer. What a view saves is the fields that are *not* touched: a caller who
needs two fields out of a sixty-field record pays for two. A caller who reads
every field of the record pays the parser's cost for each of them, and gets
close to nothing back for choosing a view. For that caller `struct_cast_le` is
the simpler call.

A view also owns nothing. It is one pointer into the caller's bytes, and that
is where its hazards come from.

## The unsafe contract

A view is the first part of the library whose API carries conditions the type
system does not check. They are repeated at every entry point and accessor in
the [reference](reference.md#views), and collected here:

- **Lifetime is wholly the caller's.** Nothing in the library or the language
  checks that the bytes outlive the view, or any sub-view or array view taken
  from it. Using a view after its bytes are gone is undefined behaviour.
- **Bounds are checked once, at creation.** At compile time for a span of
  static extent, against the span's length for one of dynamic extent. A span
  built from a wrong pointer or a wrong size cannot be detected, and the view
  trusts it.
- **Indexing an `array_of_records` is unchecked.** `[i]` with `i >= N` is a
  precondition violation. Trailing bytes are permitted after the record, so the
  read can silently return garbage rather than fail. During constant evaluation
  an index that leaves the underlying array is a compile error instead.
- **Constraints are checked once, at creation, too.** Bytes changed after the
  view exists are read as they now are, unvalidated, with no error.
- **Concurrent reads are fine, a concurrent write is not.** Any number of
  threads may read through any number of views over the same bytes, provided no
  thread writes those bytes meanwhile. That condition is the caller's, like
  lifetime.

## What a view accepts

The schema must be a **fixed layout**: every field's width, and so every
field's offset, is known at compile time. Offsets are computed once per schema
as constants, so reading a field is a pointer addition and one decode.

Fixed-width fields, magic fields, `struct_field` and `array_of_records` over an
accepted schema are viewable. `vec_field`, `str_field`, `vector_of_records`,
`maybe`, `variance` (even one whose alternatives share a width) and
`announces_byte_order` are not, because each makes some later offset depend on
the data. The [reference](reference.md#views) lists what each
field kind returns, and the reason for each rejection.

Rejection is a compile error: the call is not a viable candidate, and the
diagnostic names the rule that failed. Each name, with what it rejects, is
listed in the [reference](reference.md#the-fixed-layout-concept). The rule
applies at depth: a `maybe` inside a `struct_field`, or a `vec_field` inside an
`array_of_records` element, rejects the whole schema.

## Creating a view

The span is `std::span` of `char`, `unsigned char` or `std::byte`, const or not.
Creation does two things, and both happen once:

1. **The length.** A static extent shorter than `wire_width<S>` does not
   compile. A dynamic extent shorter than it fails with `buffer_exhaustion`,
   and `failed_at` names the first top-level field that does not fit. Trailing
   bytes after the record are fine.
2. **The constraints.** Every constrained field, magic fields included, is
   decoded and checked, in wire order. The first failure is a
   `validation_failure` naming the top-level field that contains it. An
   unconstrained field is not decoded at all. A schema with no constraint
   anywhere pays for the length check and nothing else.

Creation succeeds or fails as a unit; there is no partly validated view. A
failure inside a nested record names the outermost field, as it does for
[reading](reading.md#read-errors).

## Reading, stepping and nesting

`v["name"_f]` returns a `const` value, so `v["arr"_f] = ...` does not compile.
A `struct_field` comes back as `struct_view<Nested, ...>`, which is the same
type a top-level view of that schema has, so one function accepts either. An
`array_of_records` comes back as an `array_of_records_view`, with `size()` and
an unchecked `operator[](std::size_t)` returning a sub-view.

`wire_width<S>` is the record's width in bytes on the wire, which is not
`sizeof`. It is available before any view exists, which is what a caller
stepping through a file needs: when each record is followed by a payload whose
length is one of its fields, the next record starts `wire_width<header> +
incl_len` bytes further on.

## A worked example

<!-- docs: test/doc_examples/guide_views_example.cpp -->
```cpp
#include "s2s.hpp"

#include <fstream>
#include <iterator>
#include <span>
#include <vector>

using namespace s2s_literals;

using u16 = unsigned short;
using u32 = unsigned int;

// The 24-byte header of a classic pcap capture file. A file written by a
// little-endian host starts with the bytes d4 c3 b2 a1, so it is viewed with
// struct_view_le.
using pcap_file_header =
  s2s::struct_field_list<
    s2s::magic_number<"magic", u32, 4_B, 0xa1b2c3d4>,
    s2s::basic_field<"version_major", u16, 2_B>,
    s2s::basic_field<"version_minor", u16, 2_B>,
    s2s::basic_field<"thiszone", u32, 4_B>,
    s2s::basic_field<"sigfigs", u32, 4_B>,
    s2s::basic_field<"snaplen", u32, 4_B>,
    s2s::basic_field<"network", u32, 4_B>
  >;

// The 16-byte header in front of every captured packet. incl_len says how many
// payload bytes follow, which is why the walk below cannot be a fixed stride.
using pcap_record_header =
  s2s::struct_field_list<
    s2s::basic_field<"ts_sec", u32, 4_B>,
    s2s::basic_field<"ts_usec", u32, 4_B>,
    s2s::basic_field<"incl_len", u32, 4_B>,
    s2s::basic_field<"orig_len", u32, 4_B>
  >;

constexpr unsigned char capture_bytes[] = {
  0xd4, 0xc3, 0xb2, 0xa1, 0x02, 0x00, 0x04, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
  // first packet: 3 bytes captured of 60
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x03, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00,
  0xaa, 0xbb, 0xcc,
  // second packet: 5 bytes captured of 5
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x05, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
  0x01, 0x02, 0x03, 0x04, 0x05
};

auto write_capture(const char* path) -> bool {
  std::ofstream out(path, std::ios::out | std::ios::binary | std::ios::trunc);
  out.write(reinterpret_cast<const char*>(capture_bytes), sizeof(capture_bytes));
  return static_cast<bool>(out);
}

// Adds up the captured bytes of every packet. Only incl_len is read from each
// record header, so the other three fields are never decoded; the payload is
// skipped by arithmetic, never looked at.
auto captured_bytes(std::span<const char> file) -> std::expected<u32, s2s::cast_error> {
  u32 total = 0;
  std::size_t offset = s2s::wire_width<pcap_file_header>;
  while(offset < file.size()) {
    const auto record = s2s::struct_view_le<pcap_record_header>(file.subspan(offset));
    if(!record)
      return std::unexpected(record.error());

    const u32 incl_len = (*record)["incl_len"_f];
    total += incl_len;
    offset += s2s::wire_width<pcap_record_header> + incl_len;
  }
  return total;
}

auto main() -> int {
  if(!write_capture("capture.pcap"))
    return 1;

  std::ifstream file("capture.pcap", std::ios::in | std::ios::binary);
  // The view borrows these bytes: data has to outlive every view made from it.
  const std::vector<char> data{std::istreambuf_iterator<char>(file),
                               std::istreambuf_iterator<char>()};

  // A dynamic extent: the length is checked once, here, and so is the magic.
  const auto header = s2s::struct_view_le<pcap_file_header>(std::span<const char>(data));
  if(!header || (*header)["network"_f] != 1 || (*header)["snaplen"_f] != 0xffff)
    return 1;

  const auto total = captured_bytes(data);
  if(!total || *total != 8)
    return 1;

  // A capture cut off inside a record header names the field that does not fit.
  const std::span<const char> cut_off(data.data(), s2s::wire_width<pcap_file_header> + 16 + 3 + 10);
  const auto failed = captured_bytes(cut_off);
  if(failed.has_value())
    return 1;

  return failed.error().failure_reason == s2s::error_reason::buffer_exhaustion
      && failed.error().failed_at == std::string_view{"incl_len"} ? 0 : 1;
}
```

The magic number is checked when the file header view is created; the
`incl_len` of each record header is the only field of it that is ever decoded.
`captured_bytes` hands out nothing that outlives `data`, which is the shape that
keeps the lifetime condition easy to see.

## Nested records and arrays of records

A `struct_field` and an `array_of_records` do not copy the nested bytes; each
returns a view onto them, at a constant offset from the parent's.

```cpp
using section_header = s2s::struct_field_list<
  s2s::basic_field<"name", u32, 4_B>,
  s2s::basic_field<"size", u64, 8_B>
>;
using table = s2s::struct_field_list<
  s2s::basic_field<"count", u16, 2_B>,
  s2s::array_of_records<"sections", section_header, 4>
>;

// v: the result of struct_view_le<table>(bytes)
const auto sections = (*v)["sections"_f];   // array_of_records_view: lifetime is the bytes'
const auto size = sections[2]["size"_f];    // unchecked: 2 < sections.size() is the caller's
```

`sections[i]` with `i >= sections.size()` is not diagnosed. It computes an
address past the array and reads whatever is there.

## Checking a program that uses views

A view builds its reads from byte-wise copies, never from a typed pointer into
the buffer, so a buffer at an odd address is read correctly. x86 does not trap
on a misaligned typed load, so the project's tests prove this under the
undefined-behaviour sanitizer:

```sh
CXX=g++-14     cmake --workflow --preset ubsan --fresh
CXX=clang++-19 cmake --workflow --preset ubsan --fresh
```

Run from the repository root; it configures, builds and runs the `run_time` test
label in `build-ubsan/`, with every report fatal. CMake 3.25 or newer is
needed. A program of one's own that holds a view over a buffer is better off
running under `-fsanitize=undefined,address` too: nothing in the library
notices a view outliving its bytes, and AddressSanitizer is the tool that can.
