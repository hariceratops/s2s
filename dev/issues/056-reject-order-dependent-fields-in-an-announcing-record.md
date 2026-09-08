# [feat] reject order-dependent fields inside an announcing record

Every field read *before* the order is known must be order-agnostic or of a
statically fixed known order, since the announcing record is parsed before its
own deduction runs. The spec requires this be enforced at compile time rather
than documented as a caller obligation.

The boundary is load-bearing and narrower than it first looks: the restriction
covers the announcing record's own fields, up to and including the one that
completes resolution — **not** the record that contains it. A later sibling of
the containing record is read after resolution and carries no restriction. This
is exactly why the resolution rule is stream-position-based, and getting the
boundary wrong in either direction breaks a real format.

The check needs a depth-first walk. `include/field_list/field_list_metadata.hpp`'s
`generate_*` metafunctions operate on one flat pack, and an announcing record's
fields are not guaranteed to be flat.

Spec: `dev/specs/byte-order-is-data-not-a-template-parameter.md`.

## Acceptance Criteria
- An announcing record containing a field that is neither order-agnostic nor of
  statically fixed order fails to compile.
- The check walks into nested field lists depth-first, so a violating field
  nested inside a record inside the announcing record is caught.
- The check's scope is the announcing record only. A test proves an
  order-dependent later sibling of the *containing* record compiles and reads
  correctly — the check must not over-reach.
- A `must_not_compile` case covers the violation, with a diagnostic
  distinguishable from an ordinary type error.
- Byte arrays and other order-agnostic spellings of a marker are accepted,
  including a magic declared as bytes rather than as an integer.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.

Depends on 054.
