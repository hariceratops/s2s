# [feat] the write path synthesises the sentinel terminator

The vector written by a `until_field_equals`-terminated field holds only real
elements — 069 established that the terminator is consumed and discarded on
read, so the count means what the format means. That decision moves the cost
onto write: nothing in the caller's data says where the run ends, so the
engine has to produce the terminator itself or the file cannot be read back
at all.

There is nothing to invert and nothing to derive from elsewhere on the wire —
unlike a `len_from_field`, there is no length slot to fill; the vector's own
size is the count of real elements, full stop. The synthesis is mechanical
once 070's soundness check has ruled out any schema where it wouldn't work:
default-construct an element of the record type, set its named field to the
sentinel value, and write it after the last real element. Because that named
field drives every other field's width (070's guarantee), the synthesised
element comes out exactly the size the format expects — for the GIF case,
one zero byte.

This slice covers vectors that contain no element matching the sentinel.
Rejecting one that does is 072, and the two are deliberately not merged for
the same reason the companion kept its delimiter-in-value rejection separate
from its happy-path write: between them, the branch that writes such a vector
whole would read back short with nothing anywhere raising an error.

Spec: `dev/specs/a-record-sequence-cannot-end-at-a-sentinel.md`.

## Acceptance Criteria
- Writing a `vector_of_records<..., until_field_equals<"field", value>>` whose
  elements all have a named field different from the sentinel emits every
  real element followed by one synthesised terminator.
- Writing an empty vector emits only the synthesised terminator, and reading
  it back yields an empty vector — the empty-run round trip 069 made valid on
  read closes here.
- GIF sub-block runs and DNS label lists both round-trip byte-identically:
  written, read back, and compared against the original vector and the
  original wire bytes.
- `stream_cast_le` and `stream_cast_be` are unchanged in signature and
  behaviour.
- Covered in both forms: a `ut` suite against the constexpr stream and a
  GoogleTest case against a real `ofstream`, as a `test/schema/` pair on that
  directory's `<feature>_write.cpp` / `_ct.cpp` convention.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.
