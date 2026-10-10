# [feat] a delimited read produces a vector of trivials

062 gives `until<>` to `str_field`. This extends it to `vec_field`, so a run of
trivials ending at a sentinel value is expressible — GIF's and DNS's shapes
aside, this is the form a delimited run of bytes takes when the value is not
text.

This also closes an open question in the companion brief
(`dev/inbox/a-record-sequence-cannot-end-at-a-sentinel.md`). That feature asked
whether sentinel-terminated runs of trivials belonged to it or here; landing
them here answers it, on the grounds that brief itself gave — two ways to spell
one thing is worse than one.

`std::string` and `std::vector<T>` are both `variable_sized_buffer_like`, and
`read_impl`'s variable-sized overload (`read_impl.hpp:67`) already serves both,
so the read loop from 062 should need no second implementation. What needs
deciding is the element type. A delimiter is a byte; matching one against a
`u32` element raises partial-match and alignment questions that no format in the
scan asks for. The spec leaves this open and names restricting to byte-width
elements as the likely answer — settle it here, and if the restriction is
adopted, a wider element type must be a compile error with a legible message
rather than a silent misread.

Spec: `dev/specs/the-size-axis-cannot-say-read-until-a-byte.md`.

## Acceptance Criteria
- `s2s::vec_field<"data", u8, s2s::until<u8{0}>>` declares a field that reads
  bytes up to and including the first `0`.
- `vec_field`'s `requires variable_size_like<...>` clause
  (`field_descriptors.hpp:66`) widens the same way `str_field`'s did in 062.
- The element-type question is decided and the decision is recorded in
  `dev/design/the-size-axis-cannot-say-read-until-a-byte.md`: either the form
  accepts any `field_containable` element, or it is restricted to byte-width
  elements and a wider one fails to compile with a diagnostic naming the reason.
- If restricted, the rejection has a `test/must_not_compile/` entry.
- Every criterion from 062 holds for the vector form: delimiter consumed and not
  stored, empty value valid, `buffer_exhaustion` versus `delimiter_not_found`
  distinguished, bound counted on the value.
- The read loop is shared with 062's, not duplicated for the vector case.
- Covered in both constexpr-stream and runtime forms.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.
