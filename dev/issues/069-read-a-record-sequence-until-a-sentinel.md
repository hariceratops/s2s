# [feat] read a record sequence until a sentinel element

`vector_of_records` takes its element count from the size axis — a literal, a
sibling field, or a callable — and today's `read_field` resolves that count,
checks the footprint, `resize(n)`s, and reads exactly n elements. Some formats
supply no count at all: a GIF image's data is a run of length-prefixed
sub-blocks terminated by a zero-length sub-block, and DNS label lists have the
identical shape. The run's length is known only on reaching the thing that
ends it.

This slice adds `until_field_equals<"field", value>`, a termination form
`vector_of_records` accepts in place of a size, and inverts the read the same
way the companion delimiter feature inverted `str_field`/`vec_field`'s: with
no count there is nothing to `resize` by, so the loop becomes
read-element-then-test, and the run's length becomes an outcome of the read
rather than an input to it. Growth is therefore incremental — `push_back` per
element rather than one `resize(n)` — and the allocation bound is re-checked
after each element against the accumulated `n * sizeof(element)`, the
in-memory footprint, not wire bytes. That denominator is deliberate and
already the one settled for bounded vectors of records generally: a record
carrying variable-length subfields has no statically known wire size.

The termination form is classified by its own trait rather than folded into
an existing size category — the companion's `is_delimited_size` set this
precedent for the same reason: a size form that resolves a count before the
read and one that discovers its length during the read are different things,
and conflating them means encoding a special case into code that assumes the
count-first shape everywhere else.

This slice is read-only, and covers only the happy path. The two compile-time
misuse rejections (a declaration that cannot synthesise, and a declaration
combining `until_field_equals` with a size-axis count) are 070. Write-side
synthesis is 071, and the write-side rejection of a value that cannot
round-trip is 072.

Spec: `dev/specs/a-record-sequence-cannot-end-at-a-sentinel.md`.

## Acceptance Criteria
- `s2s::vector_of_records<"blocks", sub_block, s2s::until_field_equals<"size", u8{0}>>`
  declares a field that reads `sub_block` elements until one whose `"size"`
  field equals `u8{0}`.
- The read loop is read-then-test: read one element, test its named field
  against the sentinel; if it matches, discard the element and stop; otherwise
  `push_back` it and read the next.
- The terminating element is consumed from the stream and does **not** appear
  in the resulting vector — the element count means what the format means.
- The allocation bound (`max_bytes<N>` if declared, else `default_max_bytes`)
  is checked against `n * sizeof(element)` after every `push_back`, not once
  up front — there is no count to check up front against.
- A terminator in first position yields an empty vector, consuming the
  terminator's bytes, and is not an error.
- The stream running dry before a sentinel element is found reports
  `buffer_exhaustion`, which already describes that fact correctly.
- The bound being reached with no sentinel found reports a new
  `sentinel_not_found`, appended to `error_reason` (`cast_error.hpp`) rather
  than inserted, keeping existing enumerator values stable.
- The named field's type is equality-comparable with the sentinel value; the
  sentinel is a compile-time constant of any structural type the named field
  can hold — not restricted to integral types, even though both acceptance
  witnesses below happen to be integral.
- GIF sub-block runs and DNS label lists both parse correctly from real wire
  bytes, including the empty-run case for each.
- No change to `stream_traits.hpp` — no operation is added to the stream
  concept, and every existing custom stream compiles untouched.
- Covered in both forms per the project's standing override: a `ut` suite
  against the constexpr stream and a GoogleTest case against a real
  `ifstream`, as a `test/schema/` pair on that directory's
  `<feature>_read.cpp` / `_ct.cpp` convention.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.
