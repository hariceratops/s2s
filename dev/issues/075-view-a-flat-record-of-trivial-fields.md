# [feat] view a flat record of trivial fields

`struct_cast` decodes every field of a record before the caller sees any of
it. A scan over a million pcap headers filtering on two fields pays for all of
them. This slice is the walking skeleton of the alternative: a read-only, lazy
**view** over one record held in contiguous memory, decoding a field only when
it is asked for. It is restricted to a schema whose fields are all
`basic_field`, because that is the smallest schema for which the new thing —
a field offset known at compile time — exists at all.

The offset is what the library has never computed: everything today advances a
cursor, and nothing asks where field *k* begins. For an all-fixed schema it is
a compile-time prefix sum over the declared widths. Whether that is a constexpr
offset array over a schema value or template recursion per accessed field is
the design's call (`dev/design/<slug>.md`); size-axis value lowering has landed
(043–045) and this work is not gated on a full value IR.

Every read copies the field's bytes into a correctly typed local and
`bit_cast`s it — `memcpy`-like at run time, a byte array during constant
evaluation, the split `read_native_impl` already runs — and never forms a typed
pointer or reference into the buffer. That is the whole alignment story: no
requirement on the buffer's address or any field offset.

The fixed-layout rule first appears here as a named concept constraining the
entry point, accepting `basic_field` only; later slices widen what it accepts
and 080 adds the negative side. It contains no `static_assert`, so it can
evaluate to `false`.

Constraints are 077, arrays and strings 076, nested records 078 and 079,
rejection of unviewable schemas with named diagnostics 080, the alignment and
laziness tests 081 and the documentation 082.

Spec: `dev/specs/reading-two-fields-costs-a-whole-record.md`.

## Acceptance Criteria
- A pair of entry points, one little-endian and one big-endian (names left to
  the design), takes a read-only `std::span` of `std::byte`, `unsigned char`
  or `char` — static or dynamic extent — and returns
  `std::expected<view, cast_error>`. Byte order is fixed at the call and
  separate from `struct_cast`.
- Bytes past the record's width are ignored, so a view can sit at the front of
  a larger buffer.
- A static extent shorter than the record is a compile error. A dynamic extent
  shorter than the record fails creation with `buffer_exhaustion`.
- The record's wire width is a public compile-time constant.
- `view["name"_f]` returns a `const` value for `basic_field`, including narrow
  `N_B` integers (placing the bytes within the wider type exactly as the parser
  does), floating-point fields, and foreign byte order (swapped on read).
- For every field of every schema the tests use, the view's value equals what
  `struct_cast_le`/`_be` produces from the same bytes — at compile time and at
  run time.
- Every field offset is a compile-time constant; no offset is computed at run
  time on access. Nothing allocates in creation or in a read.
- A view is a cheap, copyable, non-owning value; no typed pointer or reference
  into the buffer is formed anywhere.
- The fixed-layout rule is a named concept with no `static_assert`, and it
  constrains the entry points directly. `static_assert(concept<accepted>)`
  holds for the test schemas in the `ut` suite.
- Covered in both forms per the project's standing override: a `ut` suite
  against a `constexpr` buffer and a GoogleTest case over bytes read from a
  real file, as a `test/schema/` pair on that directory's
  `<feature>_read.cpp` / `_ct.cpp` convention. The run-time cases join the
  UBSan configuration from 074.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.
