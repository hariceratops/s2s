# [feat] a discovered length feeds a computed one

The spec names this the criterion that matters, because it is what makes
`until<>` part of the size axis rather than a special case parked beside it. A
delimited field whose parsed value nothing else can consume is a dead end; one
that a later field's size computation can read is a member of the axis.

PNG's tEXt chunk is the witness, and it is a good one because the format forces
the composition rather than merely permitting it. The chunk holds a
NUL-terminated keyword followed by text, and the PNG spec is explicit that the
text is *not* terminated — the chunk's declared length is what locates its end.
So the text's length is the chunk length minus the keyword's discovered length
minus the one delimiter byte:

```cpp
s2s::str_field<"keyword", s2s::until<u8{0}>>,
s2s::str_field<"text", s2s::size_from_fields<remaining, "length", "keyword">>,
```

The second line is existing machinery: `size_from_fields_t` carries the callable
and its source field names (`field_size.hpp:162`), and `compute_impl` invokes it
over the field list (`field_size_deduce.hpp:28-36`). The question this slice
answers is whether that path already forwards a `std::string` sibling to the
callable, or whether it assumes an integral source the way every current use of
it does.

If it already works, this slice is a test and a doc example, and that is a fine
outcome — better a named slice that turns out cheap than real work hidden in
another slice's acceptance bullet. If it does not, the fix belongs here.

Spec: `dev/specs/the-size-axis-cannot-say-read-until-a-byte.md`.

## Acceptance Criteria
- A `size_from_fields` callable can name a field declared with `until<>` as a
  source, and receives its parsed value.
- The PNG tEXt shape above parses from a real chunk: keyword and text both
  correct, with the text's length derived from the chunk length and the keyword's
  discovered length.
- The same shape round-trips — written and read back byte-identically — which
  exercises 065's delimiter emission against a computed neighbour.
- A keyword at PNG's stated 79-byte maximum works, declared as `max_bytes<79>`
  per 062's value-not-wire accounting.
- If `compute_impl` needed changing, no existing `size_from_fields` use changes
  behaviour, and the existing computed-size tests pass unmodified.
- The example is added under `test/doc_examples/` with its `<!-- docs: ... -->`
  binding, so `doc_examples_match` covers it — it is a complete program shown in
  the docs and the repo requires those to be bound.
- Covered in both constexpr-stream and runtime forms.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.
