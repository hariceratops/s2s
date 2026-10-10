# [feat] the write path emits the value and its delimiter

The write side of a delimited field is short but not free. `write_field` for a
variable-sized buffer (`field_writer.hpp:76-98`) writes `value.size()` bytes and
stops; the length lives somewhere else on the wire, in a slot the read path knows
how to find. A delimited field has no such slot — the delimiter *is* the framing,
so the engine has to emit it after the value or the field cannot be read back at
all.

There is nothing to invert and nothing to derive. Unlike `len_from_field` there
is no length slot to fill, so the obligation machinery in
`field_write/derived_value.hpp` is untouched, and unlike `size_from_fields` there
is no computed length to check the container against — the two branches
`write_field` currently distinguishes at lines 86-95 both fall away. The
container is the only authority.

This slice covers values that contain no delimiter. Rejecting one that does is
066, and the two should not be split across a review boundary: between them the
branch can write a value that reads back short.

Spec: `dev/specs/the-size-axis-cannot-say-read-until-a-byte.md`.

## Acceptance Criteria
- Writing a `str_field` or `vec_field` declared with `until<d>` emits the value's
  bytes followed by one `d`.
- Writing an empty value emits the lone delimiter, and reading it back yields an
  empty value — the round trip 062 made valid on the read side closes here.
- No entry is added to the derived-field obligations in
  `field_write/derived_value.hpp`; a delimited field derives nothing.
- Every case from 062 and 063 round-trips byte-identically: written, read back,
  and compared against the original value and the original bytes.
- `stream_cast_le` and `stream_cast_be` are unchanged in signature and behaviour.
- Covered in both forms: a `ut` suite against the constexpr stream and a
  GoogleTest case against a real `ofstream`, placed as a `test/schema/` pair on
  that directory's `<feature>_write.cpp` / `_ct.cpp` convention.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.

## Review 2026-10-10
- `test/schema/delimited_write.cpp:174-176,266-268`: the comment "stream_cast_le/_be's own signature and behaviour are untouched by this slice" describes a change's history, not why the code is so, and sits above unrelated tests; remove it.
