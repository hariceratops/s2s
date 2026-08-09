# PRD: Struct-to-Stream Serialization (Write Direction)

Flavor: **standard** (s2s is a public/OSS header-only library with external
consumers, a README, and a `UserGuide.md`; not a personal-scale tool).

## Overview
s2s currently only supports the read direction: `struct_cast_le`/`struct_cast_be`
parse a binary stream into a `struct_field_list` declared by a schema. The
README explicitly flags the reverse — struct back out to a binary stream —
as work in progress. This feature closes that gap: a symmetric public write
API, `struct_write_le`/`struct_write_be`, that serializes an already-populated
`struct_field_list` back to a stream, covering every field kind the read
path already supports, with the same constexpr/runtime dual-path design,
the same error-handling shape, and the same validation guarantees. Where the
schema makes it possible, this feature also closes a correctness gap the
read-only design left open: fields whose value is fully implied by other
struct data (certain lengths, certain union discriminants) become
compile-time read-only, so a caller can no longer desynchronize them from
the data they describe.

## Goals
- Public API, mirroring `struct_cast_le`/`struct_cast_be` in shape:
  ```cpp
  template <field_list_like T, output_stream_like stream>
  [[nodiscard]] constexpr auto struct_write_le(stream& s, const T& obj) -> cast_result;

  template <field_list_like T, output_stream_like stream>
  [[nodiscard]] constexpr auto struct_write_be(stream& s, const T& obj) -> cast_result;
  ```
  One entry point per byte order; each dispatches internally to a
  constexpr-buffer path or a runtime-stream path with no visible difference
  to the caller — exactly like the read side.
- Full round-trip fidelity for every existing field kind: trivials, arrays
  of trivials, arrays of records, length-prefixed vectors of trivials,
  length-prefixed vectors of records, const-sized strings, length-prefixed
  strings, optionals, unions (both `type_switch`- and `if_else_ladder`-based),
  magic strings, magic numbers, magic byte arrays. Nothing deferred.
- **Compile-time read-only derivation** for the subset of dependent fields
  that are fully invertible from other struct data:
  - Length fields targeted by `len_from_field<"id">`, where the governing
    container's `.size()` is exactly the field's value.
  - Union discriminants for `type_switch`-based `variance` fields, where the
    held alternative maps back uniquely to a `match_case` value (uniqueness
    already enforced by `variance`'s existing `requires`).

  For these, the non-const `operator[]` on `struct_field_list_impl` must
  fail to compile (enforced via a `requires` clause, not a runtime check),
  while the const `operator[]` stays usable so a caller can still read a
  parsed/derived value. **This is a deliberate breaking change**: existing
  code that assigns to such a field after a read (e.g. `s["len"] = 5`) will
  stop compiling.
- **Runtime detect-and-reject** for dependent fields that are not
  invertible, where write must fail with a validation-style `cast_error`
  rather than silently deriving/correcting or trusting the caller:
  - `len_from_fields`/`size_from_fields` length targets (arbitrary user
    callable over N fields; no general inverse).
  - `maybe<... , parse_if<...>>` optional presence (presence is a predicate
    over sibling fields, not a stored flag; the struct's `std::optional`
    `has_value()` must agree with what the predicate computes at write
    time, or the write fails).
  - `if_else_ladder`-based `variance` union discriminants (branches key on
    arbitrary predicates, not a unique value).
- **Shared-length fan-out**: when N ≥ 2 variable-sized fields derive their
  size from the same single length field, write must derive/verify a
  consistent length across all of them; any contradiction is an error,
  reported the same way as other cross-field consistency failures.
- **Strict left-to-right write order**, identical to read order: a length
  field is always serialized before any field that derives its size from
  it (this is what makes deriving-then-writing the length possible at all).
- **Length-field width overflow is always an error, never silent
  truncation** — whether the length is derived (compile-time-read-only
  case) or user-supplied (runtime-checked case): if the true/derived size
  does not fit the declared width of its length field, the write fails.
- **Constraint checkers run on write**, exactly as they do on read: a
  struct holding a value that fails its field's `constraint_checker` (e.g.
  a wrong magic value) fails the write fast, rather than emitting a stream
  that won't read back.
- Complete the existing `output_stream` scaffold in
  `include/field_read/read_impl.hpp` as the basis for the write path: add
  its constexpr counterpart, and make both conform to the existing
  `write_trait`/`constexpr_write_trait` concepts in `stream_traits.hpp`.
- No heap allocation beyond what foreign-endian byteswapping already
  requires on the read side — write goes straight from the struct's own
  memory, same performance bar as read.
- Error reporting reuses `error_reason::buffer_exhaustion` for underlying
  stream-write failure by default (mirrors read's use for stream
  exhaustion) and reuses `validation_failure` where it fits. New
  enumerators are permitted only where a write-side failure genuinely
  cannot be expressed by the existing `buffer_exhaustion` /
  `validation_failure` / `type_deduction_failure` set — not introduced
  reflexively.
- Ship the documentation/build deliverables alongside the code, as part of
  this PRD's scope, not a follow-up:
  - Regenerate `single_header/s2s.hpp` via `scripts/amalgam.py`.
  - Add a "Writing to stream" section to `UserGuide.md`, mirroring the
    existing read section, and explicitly documenting the derived/read-only
    field behaviour as a breaking change from prior read-only usage.
  - Update `README.md`: flip the "read in other direction is work-in-progress"
    note, update the feature list, and check off the "Write struct to
    stream" roadmap item.

## Non-Goals
- No new field kinds or schema capabilities — this is strictly the reverse
  of the existing field-kind set declared via the current descriptors
  (`basic_field`, arrays, vectors, strings, `maybe`, `variance`, magic
  values, etc.).
- No builder/setter convenience API — the existing `operator[]` map-like
  interface (where still assignable) remains the only way to populate a
  struct before writing.
- No dry-run/size-query API and no rollback: write-once, fail-fast. A
  failed write may leave a partially written stream; the caller is
  responsible for handling that.
- Out of scope, unaffected by this feature: bitfields, read-until-delimiter,
  runtime endianness handling, modules, `struct_view` (zero-copy views),
  asynchronous read-write, seeking support, and the compile-time
  `struct_field_list` query/extend/alignment-and-padding helpers.
- No combined input/output stream concept — `output_stream_like` stays
  fully independent of `input_stream_like`; a stream type may implement
  write without read, or vice versa.
- No formal versioning/CHANGELOG process introduced by this feature; API
  stability remains informal ("get the signature right the first time").

## User Stories
- As a library consumer who already reads binary formats with s2s, I want
  to write a populated `struct_field_list` back to a stream in the byte
  order my schema declares, so I can round-trip data without hand-rolling
  serialization.
- As a schema author, I want fields whose value is fully determined by
  other fields (invertible lengths, `type_switch` discriminants) to be
  impossible to set inconsistently through the map interface, so a whole
  class of "forgot to update the length field" bugs becomes a compile
  error instead of a corrupted stream.
- As a consumer using non-invertible dependent fields (arbitrary presence
  predicates, `if_else_ladder` unions, multi-field length computations), I
  want inconsistent data caught at write time with a clear error, so I
  never silently emit a stream I can't read back.
- As an embedded/constexpr user, I want the same compile-time write path
  available for constexpr streams that already exists for constexpr reads,
  so I can validate serialization entirely at compile time, matching the
  existing constexpr read example in the README.

## Technical Approach
- **Language/runtime**: C++23, header-only, under `include/`, following the
  project's existing directory shape (a write-side mirror of
  `field_read/`, `cast/struct_cast_impl.hpp`, and the public entry point
  in `api/`) — exact file layout is an implementation decision for the
  architect, guided by the existing read-side structure.
- **Reused infrastructure**: `cast_error`/`cast_result`/`rw_result` from
  `error/cast_error.hpp`; the field/field-list/field-size/field-validation
  metadata already used by the read path; the `output_stream_like`,
  `write_trait`, `constexpr_write_trait` concepts in `stream/stream_traits.hpp`.
- **New write dispatch**: a field-kind-dispatched writer analogous to
  `field_reader.hpp`'s `read_field`, keyed on the same concepts
  (`fixed_sized_field_like`, `variable_sized_field_like`,
  `array_of_record_field_like`, `vector_of_record_field_like`,
  `struct_field_like`, `optional_field_like`, `union_field_like`), driven
  through the same field-list fold used by `struct_cast_impl`, in
  declaration order (left-to-right).
- **Derived/read-only fields**: extend the non-const `operator[]` overload
  on `struct_field_list_impl` (`include/field_list/field_list.hpp`) with an
  additional `requires` term identifying derived fields — `len_from_field`
  targets and `type_switch`-based union discriminants — computed at
  compile time from `list_metadata` by scanning the field list for
  `field_accessor` references / `match_case` uniqueness. The const overload
  stays unconstrained. Note the read path (`struct_cast_impl.hpp`)
  populates fields via `static_cast<fields&>(field_list).value`, not
  through `operator[]`, so this constraint does not affect `struct_cast`.
- **Consistency checking**: for invertible fields, the writer derives the
  value directly from the data (container size, held variant alternative)
  and never trusts a stored value. For non-invertible dependent fields, the
  writer evaluates the existing presence/type-deduction machinery
  (`field_presence_checker`, `if_else_ladder` branch predicates) against
  the struct being written and compares against the struct's actual stored
  state, failing with a validation-style error on mismatch. Shared-length
  fan-out is checked by deriving from one dependent field and verifying the
  others agree.
- **Known constraints**: no allocation beyond existing byteswap staging; no
  new external dependencies (`<expected>`, `<bit>`, existing stream traits,
  `include/lib/` vocabulary types only); new `error_reason` enumerators
  only where genuinely required.

## Success Metrics
- A new write-focused test suite exists under `test/runtime/` and
  `test/constexpr/`, mirroring the existing layout, one file per field
  kind, each asserting round-trip fidelity: populate a struct, run it
  through `struct_write_le`/`be`, then `struct_cast_le`/`be` the result,
  and assert equality with the original. Existing read tests are left
  untouched.
- A compile-fail test demonstrates that non-const `operator[]` assignment
  on a derived field (a `len_from_field` target, or a `type_switch`-based
  union discriminant) does not compile, while the const accessor still
  reads the value.
- A runtime test demonstrates that a struct with inconsistent
  non-invertible dependent data (mismatched `parse_if` presence,
  contradictory shared-length fan-out, `if_else_ladder` mismatch) fails
  `struct_write_le`/`be` with a validation-style `cast_error`, and does not
  produce a corrupted-but-unflagged stream.
- A runtime test demonstrates that a struct violating a field's
  `constraint_checker` (e.g. wrong magic value) fails to write.
- A runtime test demonstrates that a length field too narrow for the
  true/derived size fails to write rather than truncating.
- `scripts/amalgam.py` regenerates `single_header/s2s.hpp` cleanly with the
  new headers folded in; `UserGuide.md` gains a "Writing to stream"
  section; `README.md`'s WIP note, feature list, and roadmap checkbox are
  updated.

## Open Questions
- Exact new `error_reason` enumerator(s), if any, for write-side validation
  failures not expressible by the existing three — left to the architect,
  must be justified against the existing set rather than added by default.
- Exact file/header layout under `include/` for the write path — left to
  the architect, following the existing read-side directory conventions.
- Whether the compile-time-derived `type_switch` union discriminant still
  needs its own field slot in the schema (as the read path currently reads
  it as a separate field) or can be inferred purely from the variant's held
  alternative at write time — needs a design pass against
  `field_reader.hpp`'s existing `deduce_type`/`type_deduction_guide`
  mechanism before implementation.
