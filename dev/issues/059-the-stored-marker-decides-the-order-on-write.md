# [feat] the stored marker decides the byte order on write

On write the announcing field's value is already in the struct before any byte
is emitted, so the order resolves by reading it. Whatever the marker holds — from
a prior parse, or set by the caller — is the order everything after it is
written in. There is no separate requested order to derive the marker from and
none to check it against; the stored value is authoritative because there is
nothing else. This falls directly out of `struct_cast` taking no order argument.

Write-side resolution mirrors read exactly: the order takes effect the instant
the announcing field is written, at any depth, by the same rule. One rule stated
once, so the two paths cannot drift.

The marker is ordinary caller-settable struct data and **keeps its setter**. It
is not a frozen field in the sense of
`dev/specs/frozen-fields-write-their-own-value.md` — setting it is how a caller
chooses the output order. If a user does constrain a marker with `eq{v}`, that
feature applies independently and removes the setter, yielding a schema that can
only ever read and write the one order `v` decodes to. That composes; it is not
a conflict either feature special-cases.

The leaf write branch at `include/field_write/write_impl.hpp:90-95` has the same
`else`-less foreign arm as the read side and needs the same treatment.

Spec: `dev/specs/byte-order-is-data-not-a-template-parameter.md`.

## Acceptance Criteria
- `stream_cast` on a self-announcing schema writes in whatever order the
  struct's marker field holds, with no order argument anywhere in the call.
- ~~The outer host/foreign branch in `write_impl` is a runtime `if/else` with a
  total foreign arm; inner `if constexpr`s unchanged.~~ **Landed early, in 053**
  (`35b5371`). The shared `byteswap_elements` change was forced by that slice's
  float fix, and threading the runtime order through `write_impl`,
  `field_writer` and `stream_cast_impl` was done with it rather than leaving 059
  carrying an unrelated mechanical refactor. Verify it rather than re-doing it.
- Write-side resolution takes effect at the point the announcing field is
  written, mirroring read, including when the announcing record is nested.
- A TIFF-header-shaped schema round-trips through `struct_cast` and
  `stream_cast` in both the `II` and `MM` forms, byte for byte.
- The marker field keeps its `operator[]` setter; a test sets it directly and
  observes the output order change.
- A marker constrained with `eq{v}` composes with
  `frozen-fields-write-their-own-value`: no setter, one order, no error.
- `field::value` is seeded from an `eq` constraint, which is what makes that
  composition fall out rather than fail. Accepted consequence: a
  default-constructed `struct_field_list` now reads back its magic and
  `eq`-constrained fields as their values instead of zeros.
- Before relying on that, confirm against an actual build that no existing test
  asserts the old zeroed behaviour — the design's search was by reading the tree,
  not by building it (design §7.2, finding 7).
- The three write-side paths that currently read a frozen field's zeroed storage
  rather than its frozen value — `maybe`'s presence predicate at
  `field_writer.hpp:194`, a computed size, and a ladder branch — are fixed by the
  same seeding. Call this out in review: three unrelated repairs arriving inside
  a byte-order slice is something a reviewer should be told, not left to find.
- Covered in both test tiers.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.

Depends on 054, 057.
