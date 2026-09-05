# Reading two fields costs a whole record

Brief, from a design conversation on 2026-08-30. Not a PRD — the open questions
at the bottom are the interview's agenda.

## The goal

`struct_cast` parses everything. Every field is read off the stream, converted,
byte-swapped if needed, and stored, before the caller sees any of it. For a
caller that wants the whole record that is exactly right, and it is what the
library is for.

It is wrong for scanning. Walking a million pcap packet headers to filter on two
fields means paying for every field in every record to use two of them. Same for
ELF section headers, MFT entries, or any format that is a long run of small
fixed-width records where the interesting part is a predicate over a couple of
them.

The goal is a read-only view over contiguous memory that decodes a field when it
is asked for and never otherwise.

## Considerations the discussion surfaced

**It needs something the library has never computed: a field offset.**
Everything today is sequential — `deduce_field_size` answers how wide a field
is, the fold advances, the next read begins where the last ended. Nothing
anywhere asks where field k *begins*. For a schema whose fields are all
fixed-width that is a constexpr prefix sum and genuinely cheap. It is also the
only case where it exists at all.

**Which is why the schema language has to be restricted, not the implementation
made cleverer.** One `len_from_field` and every offset after it stops being a
constant. Supporting that means a runtime offset table built by walking the
record — an allocation, on the path whose entire purpose is avoiding work.
Variable-length fields are therefore out, and that is a constraint on which
schemas a view accepts rather than a limitation to be engineered around later.

**Alignment and constant evaluation have the same answer, and it is already in
the codebase.** Never form a pointer to a field. Copy its bytes into a
correctly-typed local and `bit_cast`. At runtime that is `memcpy`, which is
unaligned-safe by construction; during constant evaluation neither `memcpy` nor
`reinterpret_cast` is available, but indexing a `std::array` and `bit_cast`ing a
whole object both are. That is precisely the split `read_native_impl` already
runs between its constexpr and runtime overloads. A view does the same two
things from an offset instead of from a cursor.

**So a view defers the copy rather than avoiding it.** Per field actually
touched, it costs what the parser costs. Foreign endianness makes this
structural rather than incidental: a byte-swapped field has to be returned by
value, so there is no reference to hand out even in principle.

**Which means the saving is the untouched fields, and nothing else.** That is
real and worth having for a scan, and close to nothing for a caller that reads
one record in full. This should be stated plainly in the documentation rather
than left for people to discover — "zero-copy" is the phrase everyone will
reach for and it is not what this does.

**What it needs is contiguous memory, not a stream of known size.** For an
all-fixed schema the total width is already a compile-time constant, so a stream
reporting its size contributes nothing the schema did not already state. The
requirement is a span of at least that many bytes.

**It is a second API surface, not a strategy behind the existing one.** Different
input type, and a borrowing rather than owning contract that the language will
not check for the caller.

## Directions rejected

**Views over schemas containing variable-length fields.** Offsets after a
`len_from_field` are not constants; the runtime offset table needed to recover
them allocates, which defeats the purpose.

**`reinterpret_cast` at a computed offset.** Undefined for unaligned access and
illegal during constant evaluation. The copy-and-`bit_cast` route is both
correct and already the house pattern.

**Backing a view with a stream.** `ifstream` is not contiguous memory, and the
"known size" framing obscured that the requirement was never about size.

**Returning references into the buffer.** Impossible for a byte-swapped field
and unsound for an unaligned one, so the accessor returns by value regardless.

## Questions still open

- Eager or lazy validation? Constraints run at parse time today, and in-place
  validation is one of the library's stated features. A view does not parse.
  Validating eagerly walks every field and hands back most of the saving;
  validating on access means an unvalidated view returns whatever the bytes say
  until someone looks. These are different products, not different
  implementations, and the choice may belong to the caller.
- Is there a mutable view — assignment writing through into the buffer? Derived
  fields make that hard in general, but a fixed-layout schema has nothing that
  resizes, so it may be tractable here in a way it is not elsewhere.
- Does a view expose the same `["name"_f]` accessor as a field list? Consistency
  says yes; the fact that it returns by value rather than by reference says it
  should be visibly a different thing.
- Nested records and fixed arrays of records: does indexing one hand back a
  sub-view, or decode it whole?
- `maybe` and `variance` presumably fail the fixed-layout rule, but a `variance`
  whose alternatives are all the same fixed width arguably does not. Is that
  worth allowing, or is the simpler rule better?
- How is the fixed-layout requirement enforced, and what does the diagnostic say
  when a schema fails it? This is the first entry point that accepts a strict
  subset of the schema language.
- Should the offset table exist as a value, or be recomputed per access? An
  array of offsets is naturally constexpr code over a schema value rather than
  template recursion, which makes this the feature that most benefits from the
  value-lowering work, and possibly one that should follow it.
