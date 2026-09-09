# [feat] reject a schema with more than one announcing record

A schema containing more than one byte-order-announcing record fails to compile.

This is a **documented limitation, not a design principle**. Allowing a second
announcing record to take over from the first would fall out of the
stream-position resolution rule at no additional engine cost. It is rejected now
only to avoid committing to a resolution semantics — does the second override
the first, is nesting restricted — before a real format forces the answer.
Lifting the restriction later is additive and breaks nothing already written.

Record it alongside the deferred subtree-scoped resolution (the Exif MakerNotes
case, where an embedded marker must not leak to the rest of the file). Both
concern nested formats carrying their own marker, and a future reader should see
them as one piece of deferred work rather than two unrelated gaps.

Spec: `dev/specs/byte-order-is-data-not-a-template-parameter.md`.

## Acceptance Criteria
- A schema declaring two or more announcing records, anywhere in it and at any
  depth, fails to compile.
- The diagnostic says this is a current limitation, not an inherent conflict.
- A `must_not_compile` case covers it.
- The detection uses the same depth-first walk 056 introduced, not a second
  traversal.
- A single announcing record at any depth still compiles — a test guards against
  the check over-firing.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.

Depends on 057.
