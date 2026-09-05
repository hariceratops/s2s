# A packed byte is just a byte

Brief, from a design conversation on 2026-09-05. Not a PRD — the open questions
at the bottom are the interview's agenda.

## The goal

A protocol header's packed fields are, today, the caller's problem. IPv4's first
byte is declared as `basic_field<"ver_ihl", u8, 1_B>` and the schema says nothing
about the version and header-length nibbles inside it. The same is true of TCP's
data offset and flags, the DNS header's two flag bytes, GIF's packed logical
screen descriptor, ELF's `st_info`, and MP3's entire four-byte frame header.

For those formats the bits *are* the information. A schema that locates the byte
and stops has described the least interesting part of it, and every consumer
then writes the same shifts and masks by hand, unvalidated.

The goal is to let the schema name what is inside a packed integer.

```cpp
s2s::bit_container<"frag", u16,
  s2s::bit_field<"flags", 3_b>,
  s2s::bit_field<"offset", 13_b>
>
```

## Considerations the discussion surfaced

**There are two possible models and a scan of real formats picks one
decisively.** Byte-aligned containers — bits packed inside a whole-byte-wide
integer — cover every structural bitfield found: IPv4's version/IHL byte and its
flags-plus-fragment-offset pair, TCP's data offset and flag bits, the DNS header
flags, GIF's packed descriptor byte, ELF's `st_info`, ZIP's general-purpose bit
flag, MP3's frame header. A global bit cursor, where fields are not byte-aligned
relative to each other, is needed only by DEFLATE, JPEG entropy-coded segments,
H.264 Exp-Golomb coding, and GIF's LZW codes.

That second list is entirely compressed payloads. Those are encodings you hand
to a codec, not structure you describe declaratively — nobody wants a schema for
a Huffman bitstream. So the container model is the correct scope rather than a
simplification being settled for, and the bit-cursor cases belong to a different
kind of library.

**Nothing ever reads bits from the stream.** The container is a whole number of
bytes and is read through the existing byte-granular path into an integer.
Extraction is shift-and-mask against that local value. There is no bit boundary
to resume from, because containers are byte-aligned by construction and the next
field starts at the next byte. The "local buffer" the first pass at this worried
about is a `u16` on the stack — a value, not a buffer.

**Byte-boundary crossing stops existing.** IPv4's fragment offset is 13 bits
spanning two bytes and MP3's sync is 11 bits spanning two. Once the container is
decoded to an integer with its byte order already applied, those are
`packed & 0x1FFF` and `(packed >> 21) & 0x7FF`. Crossing is an artefact of
thinking in bytes; decode first and it disappears.

**Which is also why byte order composes instead of fighting.** Extraction is
defined over the decoded integer, never over the raw bytes, so the container is
declared with a type and read exactly like any other field.

**Masks and shifts are compile-time constants** — a constexpr prefix sum over the
declared widths, the same shape as the offset table in
`reading-two-fields-costs-a-whole-record.md`, but over bits inside one integer
rather than bytes inside a record.

**Two checks fall out of that prefix sum.** Member widths must sum exactly to the
container's bit width, since under-filling leaves bits that will not round-trip
and over-filling is nonsense. And each member's C++ type must be wide enough to
hold its declared bit width.

**MSB-first numbering.** Every format in the scan numbers from the most
significant bit.

**A `_b` literal beside `_B` is safe here, for a specific reason.** `bit_count`
and `byte_count` would be distinct types occupying distinct slots, so
`bit_field<"flags", 3_B>` and `basic_field<"x", u32, 4_b>` both fail to compile.
The `b`/`B` convention is one every reader already parses correctly, and the type
system catches a mix-up rather than letting it become a silent eightfold error.
That holds only while neither slot accepts both units.

**Do not restate the width.** The container's type gives it and the members are
asserted to fill it; a separate `2_B` would be a third statement of the same
fact, able to disagree with the other two. Same instinct as the frozen-fields
change.

**This pressures the IR far less than the TMP-to-reflection notes assumed.**
Those notes called bitfields the largest descriptor change on the grounds that
"offsets stop being byte-granular". Under the container model they do not.
Top-level fields are untouched; what is added is a field kind whose *members*
carry bit offsets. Combined with the resumable engine being held, that may leave
nothing gating the value lowering at all.

## Directions rejected

**A global bit cursor.** Only compressed payloads need it, and those are a
codec's job. Supporting it would make every field's offset bit-granular
throughout the library to serve formats it should not be parsing.

**Storing the container's bytes and extracting on demand, with no named
members.** Loses constraints on bit fields, loses the map interface, and loses
symmetry with the write path. Its one real merit is that bits nobody named
round-trip for free — recoverable by declaring a `reserved` member, and being
forced to account for every bit is usually what a wire format wants anyway.

**A separate byte-level field per bit-width field.** Breaks the moment two
fields share a byte, which is the normal case rather than the exception.

**Containers wider than 64 bits.** You run out of integer types, and
byte-boundary crossing comes back with them. Every packed structure in the scan
is one, two, or four bytes.

## Questions still open

- Do bit members take constraints? `any_of` on a 3-bit enum is obviously wanted,
  and `eq` raises a sharper question — the frozen-fields work means an
  `eq`-constrained member should write itself, which has to happen during the
  container's assembly rather than at a stream write.
- Can a bit member be a length source? IPv4's IHL and TCP's data offset are both
  lengths in 32-bit words, so `len_from_field<"ihl">` pointing at a bit member is
  a real case on day one — and it needs the dependency tables to see inside a
  container.
- Is there a spelling for bits that exist but are not named — reserved or padding
  — or must every bit be declared for the widths to sum?
- Can a bit member be a union discriminant? A packed type tag selecting what
  follows is a common protocol shape.
- Should the container also be readable as a whole, for callers that want the raw
  packed value alongside the decoded members?
- Signed bit fields. Nothing in the scan asked for one, but two's complement in a
  5-bit field is expressible and the question decides whether the member type is
  restricted to unsigned.
- LSB-first numbering: does any format worth supporting number the other way, and
  if so is that a per-container option or a global one?
