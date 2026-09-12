# The size axis

The size axis answers one question: how many bytes does this field occupy on
the wire? That is not the same question as how large its C++ type is. A field
whose type is `u32` can be declared to occupy two bytes, and reading it will
produce a `u32` holding a value that arrived as two bytes.

The axis has two kinds of member. Five forms are a **value** — written directly,
with no wrapper around it — that resolves to a byte count *before* the read
happens: the length is known the moment the schema is written, or at latest the
moment an earlier sibling field is read. The sixth, `until<d>`, is not a value
at all. It names a byte rather than a count, and the length it produces is an
*outcome* of the read, discovered one byte at a time rather than known ahead of
it.

Everywhere else on this page, "the size" means one of the first five. `until<d>`
is covered in its own section below, because what its bound counts, what it
costs, and how it fails are all answered differently from the rest of the axis.

| Form | How the width is determined |
|---|---|
| `N_B` | exactly `N` bytes, fixed at compile time |
| `len_from_field<"id">` | the value currently held by the field named `id` |
| `size_from_fields<f, "a", "b">` | a callable applied to the named sibling fields |
| `len_from_fields<f, "a", "b">` | an alias for `size_from_fields` |
| `size_dont_care` | whatever the field's own nested schema occupies |

**`N_B`** is the only form a `basic_field` accepts, and it is constrained:
`field_fits_to_underlying_type` requires `N <= sizeof(T)`. Declaring a
`basic_field<"x", u16, 4_B>` is a compile error, because four bytes off the
wire cannot be delivered into a two-byte type.

**A `basic_field` that omits its size gets `sizeof(T)`**, which is what most
fields want:

```cpp
s2s::basic_field<"version", u16>          // two bytes
s2s::basic_field<"version", u16, 2_B>     // the same field, spelled out
s2s::basic_field<"truncated", u32, 2_B>   // two bytes on the wire, u32 in the struct
```

The size and the constraint are an unordered pair, so either may be given, in
either order, or neither. An entry that is neither a size nor a constraint is
rejected by a named concept that says what was expected, and giving two of the
same kind is a `static_assert`.

**`len_from_field<"id">`** takes the width from another field's value — the
length-prefix pattern. The named field must appear earlier in the schema, since
reading is strictly left to right and the length has to be known before the
field it sizes.

**`size_from_fields<f, "a", "b">`** covers everything a single field
lookup cannot express: a length in units other than bytes, a total minus a
header, a width assembled from two fields. The callable and its inputs are
described in [Computed values](computed-values.md); `len_from_fields` is the
same template under a name that reads better at a length.

**`size_dont_care`** is what the record descriptors use. It is not zero and it
is not "unknown" — it means the width is whatever the nested schema works out
to, so there is nothing to declare. It is never written by hand.

## `until<d>`: a length discovered by reading

`until<d>` says "read until this byte, and stop" — there is no count to give.
It is accepted by `str_field` and `vec_field`, and by nothing else; a `vec_field`
declaring it is further restricted to a byte-wide element type, since a
delimiter is one byte and matching it against a wider element raises questions
— partial matches, byte order — that no format needs answered.

```cpp
s2s::str_field<"name", s2s::until<u8{0}>, s2s::max_bytes<255>>
```

**The delimiter is consumed but not stored.** It occupies a wire byte without
being part of the value's logical length — the same accounting `c_str_field`
already does with its `N + 1`.

**The write side imposes a content rule, which no other size form does.** A
value containing the delimiter cannot round-trip: written whole, it would read
back short with no error raised anywhere. So writing such a value fails outright,
under its own reason (`found_delimiter_in_value`, [Errors](../errors.md)) rather
than reaching the wire at all. No other size form constrains what a value may
contain — a `len_from_field` or `size_from_fields` field is written byte for
byte regardless of its contents — so this is worth knowing before it is
discovered by a value that happens to contain a stray delimiter byte.

**The bound changes role.** For every other size, `max_bytes` guards against a
corrupt *declared* length before anything is allocated by it. Here there is no
declared length to guard: the bound *is* the termination condition, and a field
with no delimiter anywhere ahead of it reads all the way to the ceiling before
failing. Critically, the bound counts **the value, not the wire** — `max_bytes<79>`
admits a 79-byte value *plus* its delimiter, i.e. 80 bytes off the wire, which
is what lets a schema author transcribe PNG's own stated 1–79 keyword range
without adjusting it by one.

**The cost is per byte, and it is the only field kind that pays it.** Every other
size resolves to a count and then reads that many bytes in one call; a delimited
field is read and matched one byte at a time, because there is nothing to
`resize` by until the delimiter turns up. This is proportional to the field, and
every delimited field this axis was designed against — PNG's keyword, ELF and
Git headers — is tens of bytes, so the cost is real but small. The identified
fast primitive is a `read_until(dest, delim, max)` stream operation, which would
let a stream scan its own buffer with one call instead of one byte at a time —
recorded here so a later optimisation does not have to re-derive it, and
deliberately not built now: adding it to the stream concept is additive, but it
would multiply the test matrix (a fast path and a fallback, each in constexpr
and runtime form) for one field kind, which is not worth it yet.

**Three read outcomes, one write outcome.** The delimiter turning up is success.
The stream running dry before it does is `buffer_exhaustion` — the same fact
reported the same way as anywhere else in the library. The bound being reached
first, with no delimiter among the bytes read, is `delimiter_not_found` — a
*new* reason, because truncated and corrupt are different facts about a file and
a caller can act on the difference. On the write side, a value containing the
delimiter is `found_delimiter_in_value`. All four are covered in
[Errors](../errors.md).

**An empty value is valid.** A delimiter in first position yields an empty value,
having consumed exactly one byte — ELF's string-table index 0 is exactly this.
Writing an empty value emits the lone delimiter and nothing else. A minimum
length, if a format wants one, is a constraint the schema author writes; the
size form imposes none.

<!-- docs: test/doc_examples/guide_delimited_size_example.cpp -->
```cpp
#include "s2s.hpp"

#include <fstream>

using namespace s2s_literals;

using u8 = unsigned char;

// An ELF string table is a run of NUL-terminated names with nothing else
// framing them; a symbol names its entry by a byte offset into this table, not
// by a length. Nothing on the wire says how long any one name is — the
// delimiter is the only thing that does, and max_bytes is the only bound on
// the search.
using elf_string_table_entry =
  s2s::struct_field_list<
    s2s::str_field<"name", s2s::until<u8{0}>, s2s::max_bytes<255>>
  >;

auto main() -> int {
  {
    std::ofstream out("strtab_entry.bin", std::ios::out | std::ios::binary | std::ios::trunc);
    out.write("_init\0", 6);
  }

  std::ifstream file("strtab_entry.bin", std::ios::in | std::ios::binary);

  const auto parsed =
    s2s::struct_cast_be<elf_string_table_entry>(file)
      .transform([](const elf_string_table_entry& entry) {
        return entry["name"_f] == "_init";
      });

  return parsed.value_or(false) ? 0 : 1;
}
```

A discovered length can also feed a computed one — PNG's tEXt chunk is worked
through in [Computed values](computed-values.md).

## Sizes and ceilings are different things

A size says how many bytes a field occupies; a ceiling says how many it is
allowed to occupy before the read gives up. They travel together in the same
option pack but answer different questions, and only the container descriptors
take a ceiling at all. For every size but `until<d>` that distinction is total —
see [Allocation limits](../reading.md#allocation-limits). For `until<d>` the two
collapse into one, as covered above.

## Invertible sizes, and why the write path cares

The distinction that matters on the write path is whether a size can be run
backwards.

`len_from_field<"n">` can. If the container holds four elements then `n` is
four, so the library **derives** `n` from the data and makes assigning to it a
compile error. The two can never disagree because only one of them is data
anyone supplies.

`size_from_fields<f, ...>` cannot. `f` is an arbitrary callable; knowing its
output does not reveal its inputs. So its source fields stay assignable and the
library **verifies** instead: it applies `f` at write time and fails with
`found_contradicting_length` if the answer disagrees with the container's real
size.

Reading is unaffected by the distinction — both forms simply produce a width.
[Writing](../writing.md) covers the consequences in full.

`until<d>` is neither. There is no length slot for it to derive, and nothing to
verify against — the delimiter is carried in the field's own declaration rather
than in a sibling, so a delimited field depends on nothing else and nothing
depends on it. Its only write-side rule is the content scan covered above.

## `size_choices` is not currently declarable

`size_choices` exists to give a union alternative its own width, and the
library's internal union machinery uses it. It cannot currently be written into
a schema: `is_selectable_size_v` reads the wrong trait, so the concept that
would admit a `size_choices` rejects it and admits fixed sizes instead. See
`dev/issues/027-is-selectable-size-trait-reads-wrong-trait.md`. It is listed
here for completeness and should be treated as unavailable until that is fixed.
