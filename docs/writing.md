# Writing

```cpp
template <struct_field_list_like T, output_stream_like S>
[[nodiscard]] auto stream_cast_le(S& stream, const T& obj) -> std::expected<void, cast_error>;

template <struct_field_list_like T, output_stream_like S>
[[nodiscard]] auto stream_cast_be(S& stream, const T& obj) -> std::expected<void, cast_error>;
```

These mirror `struct_cast_xx`: the same schema drives both directions, the
suffix picks the byte order of every member, and failures come back as the same
`cast_error`. There is nothing to return on success, so the `expected` holds
`void`.

Both entry points are named after what they produce, which is what makes the
pair read as one API: `struct_cast` yields a struct, `stream_cast` yields a
stream.

The struct is written strictly left to right, one field at a time, in
declaration order and at every nesting level — the same traversal
[Reading](reading.md) describes.

What is specific to this direction is that the schema now has to *produce* the
values it previously only consumed. Every construct below is declared in
[Schema](schema/index.md); this page covers only what writing does with them.

## A worked example

<!-- docs: test/doc_examples/guide_writing_example.cpp -->
```cpp
#include "s2s.hpp"

#include <array>
#include <fstream>
#include <string>

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;

// A length-prefixed log record. The length is not data anyone supplies; it is
// a consequence of the message.
using log_record =
  s2s::struct_field_list<
    s2s::magic_byte_array<"marker", 2, std::array<u8, 2>{0x4c, 0x47}>,
    s2s::basic_field<"message_length", u16, 2_B>,
    s2s::str_field<"message", s2s::len_from_field<"message_length">>
  >;

auto main() -> int {
  log_record record{};
  record["marker"_f] = std::array<u8, 2>{0x4c, 0x47};
  record["message"_f] = std::string("disk nearly full");
  // "message_length" is never assigned. It is derived from message.size().

  std::fstream file("log_record.bin",
                    std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

  const auto round_tripped =
    s2s::stream_cast_be<log_record>(file, record)
      .and_then([&file] {
        // A file stream shares one position between reads and writes.
        file.seekg(0);
        return s2s::struct_cast_be<log_record>(file);
      })
      .transform([](const log_record& parsed) {
        return parsed["message"_f].size() == 16
            && parsed["message"_f] == "disk nearly full";
      });

  return round_tripped.value_or(false) ? 0 : 1;
}
```

## Derived fields are read-only

Some fields are not data anyone supplies — they are consequences of data that
is. Writing them from the struct would let the two disagree, producing a stream
that does not read back as what was handed in. So the library derives them,
ignores whatever the struct holds, and makes assigning to them a compile error.

Two kinds of field are derived:

| Field | Derived from |
|---|---|
| the target of a `len_from_field<"n">` size | the container's `size()` |
| the `match_field` discriminant of a `type_switch` variance | the `match_case` value of the alternative currently held |

Both are invertible, which is what makes deriving them possible at all.

The two kinds are not hidden the same way, because they differ in whether
anything else can answer the question.

**A length target has no `operator[]` overload at all.** Naming one is a
plain no-such-member error, on a const object as much as a mutable one:

```cpp
log_record record{};
record["message"_f] = std::string("disk nearly full");
record["message_length"_f];                 // does not compile
std::as_const(record)["message_length"_f];  // does not compile either
```

It is hidden rather than made read-only because a read-only accessor would
still have been a lie. `operator[]` hands back a reference, so a length kept in
step at assignment time would fall out of step the moment anyone wrote
`record["message"_f].push_back(...)` — and nothing in the type system could
catch that. The container is the only thing that knows, so the container is
where you ask:

```cpp
const auto n = record["message"_f].size();
```

The field is still on the wire and still read into the struct when parsing; it
is only the user's view of it that is gone. Nothing about the byte layout
changes.

**A discriminant stays readable, and returns the stored slot.** The non-const
`operator[]` hands back a const reference, so an assignment fails as
assign-to-read-only rather than as no-such-member:

```cpp
tagged record{};
record["body"_f] = alt_1{};
record["tag"_f] = 1;                        // does not compile
const auto t = record["tag"_f];             // compiles
const auto u = std::as_const(record)["tag"_f];  // compiles
```

What that read gives is **the stored slot, not the derived value.** Derivation
happens during the write, against the struct passed as `const`, and the result
is never written back — so on a freshly built struct the slot holds whatever it
was default-constructed to, and on one that came from `struct_cast` it holds
what was parsed off the wire.

Discriminants are treated differently from length targets only because how a
caller should reach a variance field's held alternative is still unsettled;
hiding them waits on that.

**This is a breaking change to existing read-side code.** Code that parsed a
struct and then adjusted a length field in place:

```cpp
auto parsed = *s2s::struct_cast_le<log_record>(stream);
parsed["message_length"_f] = 5;   // used to compile, no longer does
```

should assign the container instead. The length follows:

```cpp
parsed["message"_f] = std::string("short");   // the written length becomes 5
```

So does code that merely *read* one, which used to compile and no longer does.
`parsed["message"_f].size()` is the replacement, and it cannot disagree with
what gets written.

Fields that are *not* derived stay assignable, including ones that look similar:
the sources feeding a `len_from_fields<callable, ...>` callable, the siblings
feeding a `parse_if` predicate, the fields feeding a `branch` predicate inside a
`type_if_else`, and a length whose only container sits inside a `maybe` or a
union alternative. None of those can be inverted, so they remain data anyone
supplies — and are verified rather than derived.

The rule behind that list is
[invertibility](schema/size-axis.md#invertible-sizes-and-why-the-write-path-cares):
a construct the library can run backwards is derived and made read-only, and one
it cannot is left alone and checked.

## What is checked at write time

Everything the library can check without a second pass is checked *before* the
offending field's first byte is emitted.

| Inconsistency | `failure_reason` | `failed_at` |
|---|---|---|
| a constraint rejects the value, including a wrong magic value | `validation_failure` | that field |
| a `parse_if` predicate disagrees with the optional's `has_value()` | `validation_failure` | the optional field |
| a `type_if_else` or computed switch selects an alternative other than the one held | `validation_failure` | the union field |
| no branch and no `match_case` matches at all | `type_deduction_failure` | the union field |
| a derived or verified length does not fit its declared width | `validation_failure` | the length field |
| a `len_from_fields` callable disagrees with the container's real size | `found_contradicting_length` | the container field |
| two containers sharing one length field imply different lengths | `found_contradicting_length` | the length field |
| a length inside a `maybe` or union alternative disagrees while that producer is active | `found_contradicting_length` | the length field |
| the underlying stream refuses the write | `buffer_exhaustion` | the field being written |

For a violation inside a nested record, `failed_at` names the outermost record
field rather than the inner one, matching how the read path reports nested
failures.

## Write-once, fail-fast

There is no rollback and no size query. Bytes handed to a stream are gone, so
the unit of atomicity is one field rather than the whole write:

- every derivation, verification and constraint check for a field runs before
  its first byte is emitted;
- a failure attributed to field `k` therefore leaves fields `0..k-1` in the
  stream and nothing of `k` — a well-defined prefix.

The one exception is a stream that fails part way through a single field: how
much it consumed is the stream's business, and the library reports
`buffer_exhaustion`. Recovering from a partial write is the caller's
responsibility — truncate, discard, or write to a staging buffer first when
all-or-nothing is required.
