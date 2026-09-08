# [feat] an announcing record nested below the top level

054 assumes the announcing record sits at the top of the schema. The spec puts
no such restriction on it: the record may appear at any depth, following the
same semantics as the type-deduction axis.

This is where the stream-position rule earns its keep. When the announcing
record is buried inside a container, the resolved order takes effect the instant
that record's last field is read — which means it governs the *remaining sibling
fields of the container it sits inside*, not just what comes after the container
closes. Those siblings are read after resolution, so they are unrestricted; only
the fields up to and including the announcement are constrained (056).

Spec: `dev/specs/byte-order-is-data-not-a-template-parameter.md`.

## Acceptance Criteria
- An announcing record nested inside another record compiles and resolves
  correctly.
- The resolved order governs the containing record's later sibling fields, and a
  test asserts precisely this — it is the case most likely to be got wrong.
- The resolved order governs every record read after the containing record
  closes, at any depth.
- The order is resolved exactly once; nothing later in the same cast re-reads or
  re-resolves it.
- An announcement that cannot satisfy "exactly once" is a compile error: one
  reached conditionally (inside a `maybe` or behind a `parse_if`) may never be
  read at all, and one reached repeatedly (inside an array or vector of records)
  would resolve more than once. Both are rejected, with a diagnostic naming
  which case applies. See `dev/design/...-template-parameter.md` §5.2 — the
  design forces this and no issue originally enumerated it.
- A `must_not_compile` case covers each of the two rejections.
- A test covers an announcing record more than one level deep.
- Covered in both test tiers.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.

Depends on 054, 056.
