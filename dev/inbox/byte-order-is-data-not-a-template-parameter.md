# Byte order is data in some formats, and the schema cannot say so

Brief, from a design conversation on 2026-08-26. Not a PRD — the open
questions at the bottom are the interview's agenda.

## The goal

Endianness is currently chosen by which function the caller invokes:
`struct_cast_le` or `struct_cast_be`, collapsed early to a two-valued
`cast_endianness` and threaded as a template parameter. That works for every
format whose byte order is a fact about the format. It cannot express a format
whose byte order is a fact about the *file* — TIFF's `II`/`MM`, pcap's magic,
ELF's `EI_DATA` — where the bytes decide.

Today such a format cannot be described in one schema at all. The caller has to
open the file, peek at a couple of bytes by hand, and then pick which of the two
entry points to call. The schema describes the format completely except for this
one fact, and that fact is the one the schema is best placed to state.

The goal is to let a schema declare a region that resolves its own byte order,
and to have everything after it read and write in whatever order that resolved
to.

## Considerations the discussion surfaced

**The hint lives inside a header record, essentially always.** TIFF's 8-byte
header is `II`/`MM` at bytes 0-1, the magic number 42 at 2-3, and the first IFD
offset at 4-7 — the two fields the hint governs are its own siblings. pcap has
the same shape across a 24-byte header, and ELF puts the hint at `e_ident[5]`, a
byte inside an array inside the header. A free-floating hint before any record
is the exception. So the nested case is the default case and cannot be treated
as a later refinement.

**Marking the record beats marking the field.** Resolution happens at the region
boundary: the marked record is parsed in full, then a deduction over its parsed
fields yields the byte order for everything after. Two things follow. The hint's
position *inside* the header stops mattering, which makes tail-placed indicators
expressible — MAT-file v5 puts its `MI`/`IM` at offset 126 of a 128-byte header.
And the order can depend on more than one field, which a single marked field
cannot express.

**This is the type-deduction axis with a different output.** `type<match_field<"tag">,
type_switch<match_case<...>>>` resolves a variant index from a field; the same
three input forms — a match over one field, an if-else ladder, a callable over
several named fields — resolve a byte order here. `compute_t` already carries a
callable plus its required field names, and `extract_type_deduction_dependencies`
already pulls those names into the dependency table. The names resolve inside the
marked record's own field table, so no cross-record path syntax is needed.

**The constraint that makes it work.** Every field inside the marked record must
be order-agnostic or of a fixed known order, since the record is parsed before
the order is known. This is checkable at compile time and should be checked
rather than documented.

**A hint that looks order-dependent usually isn't.** pcap's magic is
`0xA1B2C3D4` or `0xD4C3B2A1` read as a `u32`, and Mach-O's is `0xCAFEBABE` or
`0xBEBAFECA` — but declared as byte arrays rather than integers they are simply
two byte sequences to match, which `any_of` handles today. "The hint is
order-agnostic" is a declaration style, not a restriction on which formats work.

**Only the outer fold needs a runtime value.** Because the marked record parses
at a compile-time-known order, recursion into it keeps its existing template
parameter. The enclosing fold carries a `cast_endianness` value, updated once
when that record completes. The write direction is easier still: the hint is
already in the struct before any byte is emitted, so it resolves upfront.

**The leaf is the one non-mechanical edit.** `read_impl` and `write_impl` branch
on `if constexpr(byte_order == host) / else if constexpr(... == foreign)`, and
the foreign arm has no `else` — for a `T` that is neither trivial nor
buffer-like it falls off the end, which is sound only because that arm is never
instantiated. Making the outer branch runtime instantiates both arms for every
`T`, so the foreign side has to become total. The inner `if constexpr`s stay.

**Compile-time effect is roughly neutral.** One leaf instantiation holding both
arms replaces two holding one each. The real halving is in the layers above the
leaf, and only for code that casts one schema in both directions — which is the
test suite far more than it is user code.

**Non-BOM mixed endianness is always encapsulation.** A scan of real formats
turned up no case of per-scalar mixing. What exists is structural: Mach-O
universal binaries pair an always-big-endian `fat_header` with little-endian
slices; pcap records follow the file's order while the captured network headers
inside them are network order regardless; ISO 9660 ships an L path table and an
M path table in one image. All of these are nesting boundaries, which is the
same boundary this feature already introduces.

## Directions rejected

**Split the field list at the hint, then dispatch `le` or `be` on the remaining
stream and remaining struct.** Rejected: the split creates a boundary that
dependencies have to cross. A field after the split carrying
`len_from_field<"count">`, where `"count"` precedes the split, lands in a tail
list whose field table does not contain that name, and
`is_dependencies_resolved` rejects it correctly. Same for a `parse_if`
predicate or a `type_switch` discriminant reading backwards. It also needs a
split metafunction, recomputed metadata for the tail, and a merge back into the
full type. It manufactures the problem it was meant to avoid, and it is more
work than threading a value.

**Marking the hint field rather than the record.** Rejected once the header case
was understood: it forfeits multi-field inference and makes tail-placed hints
inexpressible.

**Parsing the pre-hint region with a default order (big-endian) and correcting
later.** Rejected in favour of requiring that region to be order-agnostic. If it
is, the starting order is unobservable and there is no default for anyone to
learn; if it is not, the default is a coin flip.

**Per-field or per-scalar endianness override.** Proposed early, withdrawn: no
verified format needs it. Every real case of mixed endianness is a region, not a
scalar.

**ISO 9660's both-byte fields.** A 32-bit value stored as eight bytes, LE copy
followed by BE copy — a binary palindrome, required in the volume descriptors
and directory records. This is not an endianness override. On read it is a
consistency check between two halves; on write the second copy is derived from
the first. That belongs with the frozen-and-derived machinery, not on this axis.

**Whole-schema endianness chosen at runtime by the caller** — not rejected, but
deliberately separated. It rides the same engine change and should be decided
alongside, not folded in silently.

## Questions still open

- Does a resolved order escape its record or stay inside it? TIFF needs escape —
  the header's `II`/`MM` governs the whole file. Exif MakerNotes need the
  opposite: a vendor may write an internal TIFF header with its own `II`/`MM`
  that governs that region only and must not leak back. Both are real, and
  nothing has been decided about how the difference is declared.
- What happens when the marked record is itself nested more than one level deep?
  The first iteration assumes it sits at the top of the schema.
- Should a region also be able to declare a *fixed* order with no hint at all?
  That is the Mach-O fat case — big-endian header, little-endian body — and it
  is much cheaper than the hint case, since it needs no runtime value anywhere.
- What is the user-facing entry point? `struct_cast_le` and `struct_cast_be`
  still make sense for schemas without a hint. A schema with one has nothing to
  pass. Whether the caller-supplied runtime order shares that entry point is
  undecided.
- On write, what happens when the hint field's stored value disagrees with the
  order the rest of the struct was written in? Either the hint field is derived
  from a requested order, or the two are checked against each other, or the
  stored hint is authoritative and there is nothing to request.
- The order-agnostic check on the marked record needs to see every field in it,
  including through nesting. Every `generate_*` metafunction in
  `field_list_metadata.hpp` works on one flat pack today, so this may need a
  depth-first walk that does not currently exist.
- Does `size_choices_t` — the parked "todo size type for holding multiple sizes
  in case of union fields" — interact with any of this, or is it independent?
