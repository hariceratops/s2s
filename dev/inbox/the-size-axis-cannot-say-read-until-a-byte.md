# A field's length can be declared, never discovered

Brief, from a design conversation on 2026-08-30. Not a PRD — the open questions
at the bottom are the interview's agenda. Companion to
`a-record-sequence-cannot-end-at-a-sentinel.md`, which came out of the same
discussion and is deliberately kept separate.

## The goal

Every form in `field_size.hpp` yields a count *before* the read happens.
`byte_count` carries it literally, `len_from_field` reads it from a sibling,
`size_from_fields` computes it from several. `deduce_field_size` returns a
number, `read_native` calls `resize(n)` and pulls exactly n bytes.

So a field whose length is only discoverable by reading — a NUL-terminated
string with no declared length anywhere — cannot be expressed. Not awkwardly:
at all. The goal is a size-axis form that says "read until this byte", making
the length an outcome of the read rather than an input to it.

## Considerations the discussion surfaced

**Three different things get called "read until", and only one needs this.** A
scan of real formats separated them. A *delimited scalar* reads until a byte and
is what this brief is about. The *remainder of a bounded region* — PNG's tEXt
text field, ID3v2 frame content, pcap packet data — is arithmetic over a length
that is already declared, and `size_from_fields` expresses it today. A
*sentinel-terminated sequence* of records is a different construct entirely and
has its own brief.

**The real cases are undeclared NUL-terminated strings.** ELF and PE string
tables, Git loose object headers (`blob 123\0`), RIFF INFO strings, and PNG's
tEXt keyword, which the spec fixes at 1–79 bytes followed by a null separator.

**PNG tEXt is the proof that it composes.** The chunk is a delimited keyword
followed by text running to the end of the chunk — and the spec is explicit that
the text is *not* null-terminated, because the chunk's declared length locates
its end. So the second half is `size_from_fields` over `chunk_length` and the
parsed keyword, computing `length - keyword.size() - 1`. That half works today.
Adding the delimited keyword completes the chunk.

```cpp
s2s::str_field<"keyword", s2s::until<u8{0}>>,
s2s::str_field<"text", s2s::size_from_fields<remaining, "length", "keyword">>,
```

**It inverts the read rather than adding a case to it.** With no count there is
nothing to `resize` by, so the bulk read becomes incremental read-and-match. On
a real `ifstream` that is a per-byte cost no other field kind pays.

**The allocation bound stops being defensive and becomes the termination
condition.** An unterminated stream reads until the ceiling stops it, and the
ceiling is the only thing that stops it. `default_max_bytes` moves from a guard
against corrupt lengths to a structural part of how this field kind reads.

**The write path gains a rule with no analogue in any existing size form.** A
value containing the delimiter cannot round-trip: it would be written whole and
read back short, with no error raised anywhere. Writing such a value has to
fail. Nothing in the current size axis has a comparable constraint, because
every other form carries its length out of band.

**The delimiter is consumed but not stored**, which is the accounting
`c_str_field` already does with its `N + 1` — the terminator occupies wire bytes
without being part of the value's logical length.

**Nothing to invert.** Unlike `len_from_field`, there is no length slot to
derive, so this form adds nothing to the derived-field machinery.

## Directions rejected

**Read until end of stream.** Rejected on two counts. No real case survived the
scan: every format with framing carries a length, and every one without relies
on the file size, which is out-of-band information the stream deliberately does
not have. And the stream concept cannot express it — `read(dest, n) -> stream&`
is all-or-nothing with no `gcount()`, there is no `eof()`, and failure is
sticky (`memstream` sets `is_bad` and never clears it; `ifstream` sets eofbit
and failbit). Success would mean handing back a parsed value while leaving the
stream in a failed state, and a short read would fill part of the destination
with no way to learn how much. Supporting it means a third operation on the
stream concept, which breaks every custom stream anyone has written — and
custom streams are documented as supported.

## Questions still open

- Single delimiter byte, or an arbitrary sequence? A sequence has to handle a
  partial match spanning a read boundary, which is a materially different
  matcher. Every verified case needs only a single byte.
- Does "delimiter not found before the stream ended" deserve its own
  `error_reason`? Reporting it as `buffer_exhaustion` is technically true and
  diagnostically useless.
- Does the write-side rejection — a value containing the delimiter — get its own
  reason, or fold into `validation_failure`?
- Byte-at-a-time reads on a real `ifstream` may be unacceptably slow. Fixing
  that means a chunked or peeking operation on the stream concept, which is the
  same breaking change that sank read-to-end. Is the slow version good enough?
- When a delimited field sits inside a region whose length is known, should that
  region's remaining byte count cap the search ahead of `max_bytes`? PNG's
  keyword is bounded by its chunk, and the spec's own 79-byte limit is tighter
  than either.
- Should the delimiter itself ever be a runtime value rather than a template
  parameter — a format where the separator is declared in a header?
