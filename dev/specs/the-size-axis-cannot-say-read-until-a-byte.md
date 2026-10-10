# A field's length can be discovered, not only declared

PRD. Supersedes the design-conversation brief this file replaced (same
filename); the brief's substance survives here, chiefly in its Open Questions
section, which was this interview's starting agenda.

Flavor: minimal — single-maintainer library, no external users of
`single_header/s2s.hpp` to coordinate with. The consumer is a schema author,
so the declaration surface is the whole product.

Companion: `a-record-sequence-cannot-end-at-a-sentinel.md`, which stays a
brief. The two came out of one discussion and are deliberately sequenced, not
merged — this one goes first because its incremental-read machinery is what
the sequence case also needs. See Non-Goals.

## Overview

Every size form in `field_size.hpp` yields a count *before* the read happens.
`byte_count` carries it literally, `len_from_field` reads it from a sibling,
`size_from_fields` computes it from several. `deduce_field_size` returns a
number and `read_native` calls `resize(n)` and pulls exactly n bytes. A field
whose length is only discoverable by reading — an undeclared NUL-terminated
string — therefore cannot be expressed at all, not merely awkwardly.

This adds one size-axis form, `until<u8{0}>`, that says "read until this byte".
It inverts the read rather than adding a case to it: with no count there is
nothing to `resize` by, so the bulk read becomes incremental read-and-match,
and the field's length becomes an outcome of the read instead of an input to
it.

## Goals

- A new size-axis form spelled `until<u8{0}>`, accepted by `str_field` and
  `vec_field`, that reads until the delimiter byte and stops.
- The delimiter is **consumed but not stored** — it occupies wire bytes without
  being part of the value's logical length, the accounting `c_str_field`
  already does with its `N + 1`.
- **Testable, read side:** PNG's tEXt chunk parses. A delimited keyword
  followed by text running to the end of the chunk, where the text is
  explicitly *not* terminated because the chunk's declared length locates its
  end:

  ```cpp
  s2s::str_field<"keyword", s2s::until<u8{0}>>,
  s2s::str_field<"text", s2s::size_from_fields<remaining, "length", "keyword">>,
  ```

  The second line works today. That the first composes with it — a computed
  size consuming a discovered one — is the criterion that matters, because it
  is what makes the form part of the size axis rather than a special case
  beside it.
- **Testable, read side:** a bare undeclared NUL-terminated string parses with
  nothing but `max_bytes` bounding the search — ELF/PE string tables, Git loose
  object headers (`blob 123\0`).
- **Testable, write side:** a value containing the delimiter fails to write,
  under its own error reason. This is the only failure mode in the feature that
  would otherwise produce a wrong answer rather than an error.
- **Testable, round trip:** every case above reads back byte-identical to what
  was written, including the empty value.
- Both a constexpr-stream form and a runtime-stream form, per the project's
  standing override.

## Non-Goals

- **Read to end of stream.** Rejected on evidence rather than deferred. No
  format in the scan needs it: every format with framing carries a length, and
  every one without relies on the file size, which is out-of-band information
  the stream deliberately does not have. The concept cannot express it either —
  `read(dest, n) -> stream&` is all-or-nothing with no `gcount()`, there is no
  `eof()`, and failure is sticky (`memstream` sets `is_bad` and never clears
  it; `ifstream` sets eofbit and failbit). Success would mean handing back a
  parsed value while leaving the stream failed.
- **Multi-byte delimiter sequences.** A sequence has to handle a partial match
  spanning a read boundary, which is a materially different matcher, and every
  verified case needs a single byte. The spelling is chosen so this can be
  added later without changing how existing schemas read.
- **Runtime-valued delimiters.** No `until_field<"sep">` form. Nothing in the
  scan declares a separator in its own header.
- **The sentinel-terminated record sequence.** `until_field_equals` on
  `vector_of_records` is the companion brief's, and gets its own PRD next.
- **A second spelling for delimited runs of trivials.** Accepting `until<>` on
  `vec_field` here settles the companion brief's open question by claiming
  them for this feature. Two ways to say one thing would be worse than one.
- *Deferred rather than excluded:* **region-bounded search**. An enclosing known
  length does not cap the search ahead of `max_bytes` — PNG's keyword is bounded
  by its chunk, and the chunk would give a better diagnostic than 16 MiB later.
  Revisit if the read path ever tracks remaining-region bytes; it carries no
  such notion today.

## Technical Approach

Hosted C++23, gcc 14+ for constexpr use. Header-only, amalgamated into
`single_header/s2s.hpp`. No new dependencies.

**Where it lands.** A new size type in `include/field_size/field_size.hpp`
alongside `byte_count` and `size_from_fields_t`, classified by the same traits;
the read in `include/field_read/`; the write-side rejection in
`include/field_write/`. Nothing to invert — unlike `len_from_field` there is no
length slot to derive, so the derived-field machinery in
`field_write/derived_value.hpp` is untouched.

**Per-byte incremental read, and no change to the stream concept.** Cost is
proportional to the field, and every delimited field in the scan is tens of
bytes — PNG caps its keyword at 79, ELF symbol names and Git headers are
similar. The pathological case, an unterminated stream running to the ceiling,
is a corrupt-file error path rather than the steady state.

**`read_until` is the identified fast primitive, recorded and not built.**
Written down so a later optimisation does not re-derive it. `peek` is *not* the
answer: `std::istream::peek()` constructs a sentry exactly as `read` does, so
peeking a byte at a time costs what reading a byte at a time costs. Peeking K
bytes at once has no `std::istream` spelling — it would need `read(K)` then
`seekg` back, which requires a seekable stream and fails on a pipe. Chunked
reading without lookahead is worse still: finding the delimiter at offset 9 of a
64-byte read means 54 consumed bytes belonging to the next field, unreturnable
without seek or unget, so a pushback buffer would have to be threaded through
every field reader's signature.

What works is pushing the scan *into* the stream: `read_until(dest, delim, max)`
lets a stream scan its own get area with `memchr` and advance exactly past the
delimiter — one call, no seek, fine on a pipe. For `std::istream` this already
exists as `getline(dest, n, delim)`, whose semantics match exactly: stops at the
delimiter, consumes it, does not store it. For `memstream` it is a loop over an
array. It would be an *additive* concept change — a `read_until`-capable concept
plus `if constexpr` dispatch keeps every existing custom stream compiling on the
per-byte fallback — so it is available whenever it is wanted. The cost that
keeps it out of this feature is testing: fast and fallback paths, each in
constexpr and runtime form, is four configurations for one field kind.

Also noted for whoever measures: in the constexpr path a per-byte loop spends
compiler evaluation steps, which are budgeted.

**The allocation bound becomes the termination condition.** `max_bytes<N>` if
declared, otherwise `default_max_bytes` (16 MiB), is the only thing that stops
an unterminated read. This is a change in kind, not degree: for every other
field the bound is a guard against a corrupt declared length, and here it is
structural — part of how the field reads. The bound counts **the value, not the
wire**, so `max_bytes<79>` admits a 79-byte keyword plus its delimiter and a
schema author transcribes PNG's stated 1–79 without adjusting.

**Three outcomes, three reports.** Delimiter found is success. The stream
running dry first is `buffer_exhaustion`, which already describes that fact
correctly. The bound being reached first is a new `delimiter_not_found` —
truncated and corrupt are different facts about a file and the caller can tell
them apart. Reporting either as `buffer_exhaustion` is technically true and
diagnostically useless.

**The write path gains a rule with no analogue in any existing size form.** A
value containing the delimiter cannot round-trip: written whole, read back
short, with no error raised anywhere. Writing it must fail, under its own error
reason rather than folded into `validation_failure`. Every other size form
carries its length out of band and so has no comparable constraint.

Both new `error_reason` enumerators are **appended**, keeping existing values
stable — the pattern `found_contradicting_length` and `excessive_length` already
set.

**An empty value is valid.** A delimiter in first position yields an empty
value, consuming one byte; ELF defines string-table index 0 as exactly this.
Writing an empty value emits the lone delimiter. A minimum length is a
constraint the schema author writes, not policy the engine imposes.

## Open Questions

- Does `until<>` on `vec_field` restrict the element type to byte-width types?
  A delimiter is a byte; matching one against a `u32` element raises partial-match
  and alignment questions that no format in the scan asks. Deciding this is
  cheap and belongs to the design, but it is not settled here.
- Naming: `until<u8{0}>` is the brief's spelling and nothing has challenged it.
  If the eventual `until_field_equals` on the sequence side makes a different
  pair read better together, this is the cheaper of the two to change — it has
  no users yet. The project's standing naming sweep (`dev/inbox/todos.md`) is
  deliberately last and will see both.
- Where the write-side delimiter check fires. A constexpr write of a literal
  could reject at compile time; a runtime value cannot. Whether both, or only
  the runtime check, is the architect's call.
