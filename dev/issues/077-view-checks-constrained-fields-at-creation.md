# [feat] creating a view checks constrained fields, with the parser's errors

A view does not parse, so nothing yet stops it handing back whatever the bytes
say. Validation is one of the library's stated features, and eager validation
of *every* field would give back most of the saving. The settled middle: at
creation, evaluate only the fields that **carry a constraint** — magic values,
`eq`, `any_of`, ranges — and decode nothing else. A pcap record header has no
constrained fields, so for it creation costs nothing but the length check.
Reads then return plain values, with no per-read unwrapping.

The error is the parser's. For a span at least as long as the record, creation
fails exactly when `struct_cast` with the same byte order fails on those bytes,
with the same `cast_error` — same `failure_reason`, same `failed_at`, including
today's limitation of naming only the outermost field for a nested violation.
Constraint evaluation reuses `field_value_constraints.hpp` over the constrained
fields only, in wire order.

A too-short span is checked first and always reports `buffer_exhaustion`; what
`failed_at` holds in that case is the design's call, because the parser could
report a constraint failure on an earlier field before running out of bytes,
so "same error as the parser" is claimed only for a long-enough span.

Nested records are 078 and arrays of records 079; this slice covers the flat
record, where the equivalence is simplest to state and test.

Spec: `dev/specs/reading-two-fields-costs-a-whole-record.md`.

## Acceptance Criteria
- The fixed-layout concept additionally accepts `magic_byte_array`,
  `magic_string` and `magic_number`, and a field carrying any constraint the
  parser supports.
- Creation evaluates every constrained field, in wire order, and nothing else:
  an unconstrained field is not decoded during creation.
- For a span at least as long as the record, creation returns an error if and
  only if `struct_cast_le`/`_be` does on the same bytes, and the two
  `cast_error`s are equal. Tested over a violation of each constraint kind, in
  both byte orders.
- A dynamic span one byte short fails with `buffer_exhaustion` before any
  constraint is evaluated.
- Changing the bytes of an unconstrained field that is never read changes
  neither creation's result nor any value read.
- Magic fields read back through `view["name"_f]` with the same values as the
  parser (`fixed_string<N>` for `magic_string`).
- Covered in a `ut` suite (compile time and run time) and a GoogleTest case
  over bytes read from a real file; run-time cases join the 074 UBSan
  configuration.
- `ctest` is green tree-wide; `single_header/s2s.hpp` is regenerated in the
  same commit.
