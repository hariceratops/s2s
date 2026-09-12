# [feat] a value containing the delimiter fails to write

This is the only failure mode in the feature that would otherwise produce a wrong
answer instead of an error, which is why it is a slice rather than a footnote on
065.

A value holding the delimiter cannot round-trip. It is written whole, and read
back short at the first embedded delimiter, with the remainder of the value
silently reinterpreted as whatever field comes next. Nothing in either direction
raises anything. The write has to fail.

No existing size form has a comparable rule, because every other one carries its
length out of band — a `len_from_field` string may contain any byte at all, since
the length slot says where it ends. Here the value's content and its framing are
the same bytes, so a constraint on content is the only thing standing between a
caller and a corrupt file.

The rejection gets its own `error_reason` rather than folding into
`validation_failure`. A schema author who hits this has not violated a constraint
they wrote; they have hit a rule of the size form itself, and the diagnostic
should say which. Appended to the enum at `cast_error.hpp:9`, like
`delimiter_not_found` in 062.

Spec: `dev/specs/the-size-axis-cannot-say-read-until-a-byte.md`.

## Acceptance Criteria
- Writing a value containing the declared delimiter fails with a new
  `error_reason`, appended rather than inserted, distinct from both
  `validation_failure` and `delimiter_not_found`.
- `failed_at` names the offending field.
- The check costs nothing on the read path and nothing for any other size form.
- A value whose *last* byte is the delimiter is still rejected — there is no
  "already terminated, pass it through" exemption, because the engine emits the
  delimiter itself in 065 and would emit a second one.
- An empty value is accepted, as 065 requires.
- The rule is stated in the docs alongside the form itself in 068, not left to be
  discovered by hitting it.
- Covered in both constexpr-stream and runtime forms.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.
