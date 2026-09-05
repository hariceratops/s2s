# A file of records has to be parsed all at once

Brief, from a design conversation on 2026-08-31. Not a PRD — the open questions
at the bottom are the interview's agenda.

## The goal

`struct_cast` returns one fully-parsed field list. That is right for a record,
and wrong for a file that *is* a long run of records. A pcap with a million
packets should not become a `std::vector<packet>` with a million entries so the
caller can look at each one and keep four.

It is worse than inefficient: such a file cannot be described as a schema at
all. `vector_of_records` needs a count from the size axis, and a header-then-
records-until-the-file-ends format has no count anywhere. So the caller writes
the loop by hand, calls `struct_cast` per record, and invents their own
termination condition — which is the part that is easy to get wrong.

The goal is to hand back the records one at a time.

## Considerations the discussion surfaced

**Two very different things share the word "coroutine", and this is the cheap
one.** A generator can *wrap* the engine or *replace* it. Wrapping means a
`std::generator<record>` holding a loop that calls the existing `struct_cast`
once per record and `co_yield`s the result — no `co_await` anywhere inside the
engine, every field read exactly as it is today. Replacing means making every
read suspendable so a parse can stop 1500 bytes into a 4 MB `vec_field`. They
differ by about two orders of magnitude in cost. This brief is the wrapper.

**Almost nothing needs building.** `std::generator` is C++23 and C++23 is
already the baseline. The parse path stays `constexpr` because it is untouched;
only the streaming wrapper is not, and nobody iterates a million records during
constant evaluation.

**A generator beats a hand-written iterator once nesting is involved.** For a
flat run the iterator's whole state is a stream position and a coroutine frame
buys nothing. But yielding from *inside* nested structure — every sub-block of
every GIF frame — gets its stack for free from the call stack, where an iterator
needs that stack hand-built and stored.

**The error belongs on the handle, not on the element.**
`generator<std::expected<record, cast_error>>` forces error handling at every
dereference and reads badly. Yield plain records; let the caller ask the
generator why it stopped once the loop ends.

**Termination is the actual design problem.** pcap is a 24-byte header followed
by records until the file ends, with no count anywhere. That is the
read-until-end-of-stream capability rejected in
`the-size-axis-cannot-say-read-until-a-byte.md`, arriving through a different
door.

**It looks resolvable without touching the stream concept.** Attempt the next
record. If the parse fails with `buffer_exhaustion` on the record's *first*
field, nothing was there and the generator is finished. If it fails on any later
field, the file is genuinely truncated and that is an error worth surfacing.
`cast_error` already carries `failed_at`, so both are distinguishable with what
exists today — no `eof()`, no `gcount()`, no break for anyone's custom stream.

**Two things to write down rather than discover.** The heuristic is a heuristic:
it reads "could not begin a record" as "no more records". And the stream is left
in a failed state once the terminating read fails — `memstream::is_bad` is
sticky, `ifstream` sets failbit — which is harmless for a generator that has
finished but surprising for a caller who checks the stream afterwards.

**This changes the sequencing argument in the TMP-to-reflection notes.** Those
notes made coroutine lazy parsing *the gate* — the feature that had to land
before a value IR could be frozen, on the grounds that a resumable engine and a
straight-line one might not want the same descriptor. A wrapper wants nothing
from the descriptor whatsoever. With the resumable engine held, bitfields is the
only remaining real pressure on `field_desc`.

## Directions rejected

**The resumable engine — the socket case.** Held deliberately rather than
dismissed. It requires `read_field`, `read_impl`, `read_native` and every nested
cast to become coroutines; partial reads on the stream concept, since suspending
mid-field means knowing how many bytes actually arrived; per-field resumption
state; and an awaitable driven by the caller's executor, which is the library
taking a position on I/O. It also gives up `constexpr` throughout the engine
rather than only at the edge. Worth its own brief if it is ever picked up.

**A hand-written input iterator.** Genuinely attractive — keeps `constexpr`,
allocates nothing, and for a flat run its state is one stream position. Rejected
because that only holds for a flat run, and the formats worth iterating are
nested.

**`generator<std::expected<record, cast_error>>`.** The error goes on the
handle.

**Extending the stream concept with `eof()` or a byte count.** Rejected here for
the same reason as in the delimiter brief: it breaks every custom stream, and
custom streams are documented as supported. The first-field heuristic exists to
avoid needing it.

## Questions still open

- Where does the repeating part begin? pcap is a 24-byte header then records;
  PNG is a signature then chunks; ELF locates its sections by offsets in the
  header. Does the generator take a header schema and an element schema, or does
  the caller parse the header itself and hand over a positioned stream?
- Is the first-field heuristic sound beyond pcap? A record whose first field is a
  magic value fails its *constraint* rather than exhausting the buffer, so a
  clean end and a corrupt record may not separate as neatly as the motivating
  case suggests.
- Should the generator expose progress — a record count or byte offset — so a
  caller can report it, or stop and resume later from a known position?
- Does this compose with views? The scanning case in
  `reading-two-fields-costs-a-whole-record.md` is the same shape: walk many
  records, read a few fields of each. A generator *of views* may be what that
  brief actually wants, and if so the two features should be designed together
  rather than discovering the overlap afterwards.
- Heterogeneous runs. PNG chunks are a sequence of differently-typed records
  selected by a type tag. Is that this feature with a `variance` as the element
  type, or something else?
- The `constexpr`-by-default override in `AGENTS.md` says stream-touching code
  provides both overloads and is covered in both forms. The wrapper cannot. Does
  that exception get written into `AGENTS.md`, and how is the boundary stated so
  it does not become a general excuse?
