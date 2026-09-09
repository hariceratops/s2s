# Byte order is data in some formats, and the schema cannot say so

PRD. Supersedes the design-conversation brief this file replaced (same
filename); the brief's substance survives here, chiefly in its "Considerations
the discussion surfaced" and "Directions rejected" sections, which this PRD
does not re-litigate, and its "Questions still open" section, which was this
interview's starting agenda.

Flavor: standard — s2s is a header-only library with external consumers
(anyone including `single_header/s2s.hpp`), a genuine second party distinct
from the maintainer.

## Overview

Byte order is chosen today by which function the caller invokes,
`struct_cast_le` or `struct_cast_be`, collapsed to a two-valued
`cast_endianness` template parameter. That covers every format whose byte
order is a fixed fact about the format, but not one whose byte order is a
fact about the file — TIFF's `II`/`MM`, pcap's magic, ELF's `EI_DATA` —
where the bytes themselves decide. Today such a format cannot be described
in one schema: the caller has to peek at the file by hand before choosing
which entry point to call.

This feature lets a schema declare a record whose fields, once parsed,
determine the byte order for everything read after it — for the rest of the
file, not just that record's own remaining siblings. The declaration reuses
the library's existing type-deduction machinery (`match_field` /
`type_switch`, an if-else ladder, or a callable over named fields via
`compute_t`) with a byte order as the output instead of a variant index.
Resolution happens at the point in the byte stream where the deciding
record's last field finishes, regardless of how deep in the schema tree that
record sits. A new entry point, `struct_cast<T>(stream)`, parses such a
schema without the caller supplying an order at all. `struct_cast_le` and
`struct_cast_be` are untouched and keep serving fixed-order formats exactly
as before.

## Goals

- A schema can mark one record as byte-order-announcing, declaring how its
  parsed fields resolve to a byte order using the same three input forms the
  type-deduction axis already supports: a `match_field` against one field's
  value, an if-else ladder, or a `compute_t` callable over several named
  fields. Field names resolve inside the announcing record's own field
  table, the same scoping type deduction already uses — no cross-record path
  syntax.
- The announcing record may appear at any depth in the schema, not only at
  the top. Resolution is a property of stream position, not schema-tree
  position: the moment the record's last field is read, the resolved order
  takes effect for every field read after that point, at any depth,
  including the remaining sibling fields of the record it sits inside and
  every record that follows once that containing record closes. Once
  resolved, the order is never re-read or re-resolved later in the same
  cast.
- Every field that is read *before* the order is known — the announcing
  record's own fields, up to and including the one that completes it — must
  be order-agnostic or of a statically fixed known order, since the record
  is parsed before the order is known. This is enforced at compile time, not
  documented as a caller obligation. Fields that come after the announcing
  record, including later siblings of its containing record, carry no such
  restriction — they are read only once the order is already known. Getting
  this boundary exactly right (narrower than "every field in the enclosing
  record") is load-bearing.
- The compile-time order-agnostic check walks into nested field lists
  depth-first to see every field of the announcing record, since
  `field_list_metadata.hpp`'s `generate_*` metafunctions operate on one flat
  pack today and the announcing record's fields are not guaranteed to be
  flat.
- A new public entry point, `struct_cast<T>(stream)`, parses a schema that
  declares an announcing record, with no order argument — the schema
  resolves its own order. `struct_cast_le<T>(stream)` and
  `struct_cast_be<T>(stream)` are unchanged in behavior and signature and
  continue to serve schemas with no announcing record.
- Calling `struct_cast` on a schema with no announcing record is a
  compile-time error naming the mismatch and pointing at `struct_cast_le`/
  `struct_cast_be` instead. Calling `struct_cast_le`/`struct_cast_be` on a
  schema that does declare an announcing record is likewise a compile-time
  error, naming the mismatch and pointing at `struct_cast`. There is no way
  to force a fixed order over a self-announcing schema.
- On write, the announcing record's field(s) are ordinary, caller-settable
  struct data — not frozen in the sense of
  `dev/specs/frozen-fields-write-their-own-value.md`. Whatever value is in
  that field when `stream_cast` runs (from a prior parse, or set by the
  caller) is the byte order everything after it is written in. There is no
  separate "requested order" to derive the field from or to check the field
  against; the stored value is authoritative because there is nothing else.
  Write-side resolution mirrors read: the order takes effect the instant the
  announcing field(s) are written, and applies to everything written after,
  at any depth, by the same rule as read.
  - Corollary: if a user constrains an announcing field with `eq{v}`,
    `frozen-fields-write-their-own-value`'s mechanism applies to it
    independently and removes its setter, always writing `v`. This is not a
    conflict between the two features — it composes to a schema that can
    only ever read and write the one order `v` decodes to, which is a
    legitimate (if degenerate) schema, not an error case either feature
    needs to special-case.
- Making the leaf read/write step (the innermost `if constexpr(byte_order ==
  host) / else if constexpr(... == foreign)` branch in `read_impl`/
  `write_impl`) total is in scope: today the foreign-order arm has no `else`
  and is sound only because it is never instantiated for types that can't
  take it. Once the outer order becomes a runtime branch instead of a
  template parameter, both arms are instantiated for every `T`, so the
  foreign arm must handle every case the host arm does. The inner
  `if constexpr`s (trivial vs. buffer-like vs. other) stay as they are.
- Acceptance is judged against TIFF's header shape: `II`/`MM` at bytes 0-1
  (order-agnostic, a 2-byte match), magic `42` at bytes 2-3 (order-dependent,
  read using the resolved order), first IFD offset at bytes 4-7
  (order-dependent, also using the resolved order) — one schema, cast with
  `struct_cast`, round-tripping through `stream_cast` in both the `II` and
  `MM` forms. This format is the target the design is checked against; it is
  not required that a TIFF example schema ship as part of this feature.
  - Note where this shape puts the announcing record's boundary: the
    announcing record is the `II`/`MM` marker alone, not the whole 8-byte
    header. Magic `42` and the IFD offset are later siblings of it inside
    that header, read after resolution has already happened — which is
    exactly the stream-position rule above, and exactly why the
    order-agnostic check is scoped to the announcing record rather than to
    its container. A schema that instead made the full 8-byte header the
    announcing record would fail that check on those two order-dependent
    fields, and would be right to.
- Build time and generated code size must not regress. The brief's own
  analysis (one leaf instantiation holding both arms replaces two holding
  one each; the real savings from today's split live in the layers above the
  leaf, which this feature collapses only for code that casts one schema in
  both directions) is the basis for this being a qualitative requirement,
  not a measured gate — no new compile-time budget or benchmark is added by
  this feature.

## Non-Goals

- A resolved order escaping only its own subtree (the Exif MakerNotes case:
  an embedded TIFF header whose `II`/`MM` governs just that region and must
  not leak to the rest of the file) is out of scope. This feature only
  implements order escaping to the rest of the file. The declaration surface
  should not be designed in a way that forecloses adding subtree-scoped
  resolution later as an opt-in alternative, but no such opt-in ships now.
- A schema containing more than one byte-order-announcing record is a
  compile-time error in this feature, not a supported case. This is a
  deliberate, documented limitation, not a design principle: allowing a
  second announcing record to take over from the first would fall out of
  the stream-position resolution rule at no additional engine cost, and
  rejecting it now was chosen only to keep that door open rather than commit
  to a resolution semantics (does the second override the first? is nesting
  restricted?) before there's a real format forcing the answer. Record this
  alongside the subtree-scoping non-goal above — both concern nested formats
  carrying their own marker, and a future reader should see them as one
  piece of deferred work.
- A region declaring a *fixed* compile-time order with no runtime hint at
  all (the Mach-O fat-binary case: always-big-endian header wrapping
  little-endian slices) is out of scope. It rides the same engine change but
  is cheaper and separable, and is left for later.
- A caller-supplied runtime byte order (the caller says "treat this as
  little-endian, don't inspect the file") sharing `struct_cast` or any other
  entry point is explicitly out of scope. It is deliberately separated per
  the brief, to be decided on its own once this engine change lands. The
  entry point and engine design here must not be shaped in a way that rules
  it out later, but nothing about it is decided or implemented now.
- `size_choices_t` (the parked idea for a size type holding multiple
  possible sizes, for union-field alternatives of different widths) is
  unrelated to this feature and is not touched by it. It sizes a different
  axis (union alternatives) and, per `dev/design/union-alternatives-have-no-
  option-pack.md`, its last internal consumers were already deleted in issue
  048 — it appears orphaned. That is a separate, independent cleanup
  candidate, not part of this work.
- Per-field or per-scalar endianness override remains out of scope, as the
  brief already settled: every verified real-world case of mixed endianness
  is a region boundary (a nested record), never a bare scalar.
- ISO 9660's dual LE/BE-copy fields remain out of scope, as the brief
  already settled: that is a consistency check between two stored copies on
  read and a derivation on write, not an endianness axis, and belongs with
  frozen/derived-field machinery, not this feature.

## User Stories

- As a schema author working with TIFF, pcap, or ELF, I want to declare the
  record that carries the file's byte-order marker directly in the schema,
  so that I no longer have to peek at raw bytes myself before choosing
  `struct_cast_le` vs `struct_cast_be`.
- As a schema author with an existing fixed-order format, I want
  `struct_cast_le`/`struct_cast_be` to behave exactly as they do today, so
  that this feature costs me nothing if I never opt into it.
- As a schema author, I want a clear compile-time error, not a runtime
  surprise, if I call the wrong entry point for my schema's shape (self-
  announcing vs. fixed-order), so that the mismatch is caught before the
  program ever runs.
- As a schema author writing a self-announcing format back out, I want the
  byte order I write in to come from the struct's own marker field, so that
  writing an `II`-order TIFF is just a matter of the marker field already
  holding `II` — no separate order to pass or reconcile.

## Technical Approach

- Language/runtime: C++23, header-only, unchanged from the rest of s2s.
- Declaration surface: extends the type-deduction axis
  (`include/type_deduction/`) — `type<match_field<id>, type_switch<...>>`,
  `type<compute_t<callable, R, field_name_list>, type_switch<...>>`, and the
  if-else ladder form — reusing `compute_t`'s existing carriage of a
  callable plus its required field names and
  `extract_type_deduction_dependencies`'s existing pull of those names into
  the dependency table, with a `cast_endianness` value as the resolved
  output type instead of a variant index. A new marker construct designates
  which record in the schema is the announcing one and carries this
  deduction; naming and exact placement (option in the record's trailing
  pack vs. a dedicated wrapper type) is an implementation decision for the
  architect, not fixed by this PRD.
- Read path (`cast/struct_cast_impl.hpp`): the enclosing fold currently
  carries `cast_endianness` as a compile-time template parameter throughout
  recursion. It becomes a runtime value threaded through the fold, updated
  exactly once, at the point the announcing record finishes parsing.
  Recursion into the announcing record itself keeps the existing
  compile-time template-parameter path, since that record parses before the
  order is known and must do so at a statically fixed or order-agnostic
  order.
- Write path (`cast/stream_cast_impl.hpp` and `field_write/`): the
  announcing field's value is already in the struct before any byte is
  emitted, so resolution happens upfront by reading that field, then
  threading the resolved value the same way as the read path from the point
  the announcing field is written onward.
- Leaf rewrite (`field_read`/`field_write` innermost order branch): the
  `if constexpr(byte_order == host) / else if constexpr(... == foreign)`
  pattern with no `else` on the foreign arm becomes a runtime `if/else`
  where both arms are real, complete implementations for every `T` — this is
  the one non-mechanical piece of new code the brief identifies.
- Compile-time order-agnostic check: a new depth-first walk over
  `field_list_metadata.hpp`'s field table for the announcing record,
  covering nested record fields, since the existing `generate_*`
  metafunctions assume one flat pack. Scope of the check is exactly the
  fields read up to and including the announcing field(s) that complete
  resolution — not the whole containing record.
- New entry point (`api/struct_cast.hpp`): `struct_cast<T>(stream)`,
  constrained (via a named concept, per project convention) to schemas
  declaring exactly one announcing record, returning
  `std::expected<T, cast_error>` like the existing entry points.
  `struct_cast_le`/`struct_cast_be` gain the complementary constraint
  (no announcing record) so misuse fails to compile. Multiple announcing
  records anywhere in the schema is also a compile-time failure, per the
  Non-Goals limitation above.
- Known constraint: this is a template-metaprogramming-heavy change to
  `field_list_metadata.hpp`, the type-deduction machinery, and the cast
  implementation layers; the compile-time-neutrality goal applies to all of
  it, qualitatively (see Goals), not measured against a numeric budget.

## Success Metrics

- A TIFF-header-shaped schema (`II`/`MM` match, order-dependent `u16` magic,
  order-dependent `u32` offset) compiles, and `struct_cast<TiffHeader>`
  correctly resolves and reads both the `II` and `MM` forms of the header.
- The same schema fails to compile against `struct_cast_le`/`struct_cast_be`,
  and a schema with no announcing record fails to compile against
  `struct_cast`, each with a diagnostic naming the mismatch and the correct
  entry point to use instead.
- `stream_cast` on the TIFF-shaped schema writes bytes matching whichever
  order the struct's marker field holds at the time of the call, for both
  `II` and `MM`, with no separate order argument anywhere in the call.
- A schema with a byte-order-announcing record nested below the top level
  (e.g., inside another record) compiles and resolves correctly, with the
  resolved order visibly governing sibling fields of the containing record
  and fields of records that follow.
- A schema with two announcing records anywhere in it fails to compile.
- An announcing record containing a field that is neither order-agnostic
  nor of statically fixed order fails to compile, with a diagnostic
  distinguishing this from an ordinary type error.
- Every existing test exercising `struct_cast_le`/`struct_cast_be` and
  `stream_cast_le`/`stream_cast_be` continues to pass unmodified.

## Open Questions

None outstanding from the interview — the brief's full "Questions still
open" agenda is resolved above:
- Escape vs. subtree-scoped: escape only, subtree-scoped deferred (Non-Goals).
- Nesting depth: unrestricted, resolution is stream-position-based (Goals).
- Fixed order with no hint (Mach-O fat case): deferred (Non-Goals).
- Entry point: new `struct_cast<T>`, existing entry points untouched, both
  directions of misuse are compile errors (Goals).
- Write-side hint-vs-order disagreement: moot — the stored field is
  authoritative, there is no separate requested order (Goals).
- Depth-first walk for the order-agnostic check: in scope (Goals, Technical
  Approach).
- `size_choices_t`: unrelated, untouched (Non-Goals).

Remaining decisions are implementation-level and belong to the architect,
not this PRD: the exact spelling of the announcing-record marker construct,
whether it is a trailing record option or a wrapper type, and the precise
diagnostic wording for the two entry-point-misuse compile errors.
