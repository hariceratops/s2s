# [feat] Length-prefixed vectors and strings with derived lengths

Support the variable-sized field kinds whose size comes from a single named
length field (`len_from_field`). Because writing is strictly left-to-right,
the length field is serialized before the data it governs — which is what
makes deriving the length from the actual container possible.

Depends on: 001.

## Acceptance Criteria
- `vec_field` (length-prefixed vector of trivials) round-trips.
- `str_field` (length-prefixed string) round-trips.
- The length field's written value is **derived** from the governing
  container's `.size()`. Any value stored in the length field is ignored, not
  trusted — deriving is unconditional, so desynchronization cannot reach the
  stream.
- The derived length is written in the length field's declared width and byte
  order.
- If the derived size does not fit the declared width of its length field
  (e.g. a `u8` length field and a 300-element vector), the write fails with a
  `cast_error` naming the length field. Silent truncation is a defect.
- Round-trip verified for both byte orders, in both `test/runtime/` and
  `test/constexpr/`; the overflow failure has its own runtime test.
- No heap allocation introduced.
