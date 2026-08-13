# [feat] Hide len_from_field targets from operator[]

A field whose value the write path derives from another field's data is not
part of the user's model of the struct — nobody supplies it, and no assignment
can keep it honest. `struct_field_list::operator[]` currently keeps such a
field visible and hands back a const reference, which is enough to reject an
assignment but leaves a value in the API that reads as stale (zero until
`stream_cast` runs) rather than as absent.

`field_list_metadata.hpp` merges length-derived and discriminant-derived ids
into one `derived_field_ids` bucket, and `field_list.hpp`'s `operator[]` gives
every entry in it the same const-only overload. The two now need different
treatment: a `len_from_field` target loses its overload entirely, while a
`match_field` discriminant keeps today's behaviour unchanged — hiding
discriminants is deferred, because how a caller should read a `variance`
field's held alternative at all is a live design question the spec does not
settle.

Whether the split is two lists or one list with a per-entry kind tag is the
architect's call; the spec requires two behaviours out of what is currently one
bucket, not a particular representation.

Independent of the size-axis work in 044 and 045 — it changes no spelling and
can land in either order relative to them.

Spec: `dev/specs/schema-api-verbosity.md`.

## Acceptance Criteria
- A `len_from_field` target has no `operator[]` overload at all: naming one is
  a plain no-such-member error, not an assign-to-const error.
- A `match_field` discriminant target still reads through `operator[]` as a
  const reference, exactly as today.
- A `test/must_not_compile/` case covers the length-target rejection.
- Round-trip tests and doc examples that read a length target are rewritten to
  compare the data field's own size instead; no new read-side affordance is
  added for this.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries.
