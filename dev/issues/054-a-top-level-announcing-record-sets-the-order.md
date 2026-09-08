# [feat] a top-level announcing record sets the byte order for what follows

The walking skeleton for the feature. A schema marks one record as
byte-order-announcing and declares how its parsed fields resolve to a
`cast_endianness`, using the `match_field` form of the existing type-deduction
axis. A new entry point `struct_cast<T>(stream)` parses such a schema with no
byte order supplied by the caller.

Scope is deliberately the thinnest end-to-end path: the announcing record sits
at the top of the schema, and only the `match_field` declaration form works.
Nesting is 057, the other two declaration forms are 055, the compile-time guard
on the announcing record's own fields is 056, and the write path is 059.

Resolution is a property of stream position, not schema-tree position: the
moment the announcing record's last field is read, the resolved order applies to
everything read after that point. It is resolved once and never re-read.

The declaration reuses `include/type_deduction/` — `match_field` against one
field's value feeding a `type_switch` — with a `cast_endianness` as the output
instead of a variant index. Field names resolve inside the announcing record's
own field table, the same scoping type deduction already uses; no cross-record
path syntax is introduced.

Spec: `dev/specs/byte-order-is-data-not-a-template-parameter.md`.

## Acceptance Criteria
- A schema can mark exactly one record as byte-order-announcing and attach a
  `match_field`-based deduction resolving to a `cast_endianness`.
- `struct_cast<T>(stream)` exists in `include/api/struct_cast.hpp`, takes no
  byte order argument, and returns `std::expected<T, cast_error>` like the
  existing entry points.
- A TIFF-header-shaped schema reads correctly in both forms: an `II`/`MM`
  announcing record followed by an order-dependent `u16` magic, where the magic
  is read using the resolved order.
- The announcing record's own fields are read before the order is known, at the
  statically fixed order the existing path uses.
- Marker bytes matching neither alternative fail as an ordinary constraint
  failure — a `cast_error` carrying `error_reason::validation_failure` and the
  field's id. Nothing special is added for this case.
- `struct_cast_le` and `struct_cast_be` are unaffected on schemas that declare
  no announcing record.
- Covered in both test tiers.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.

Depends on 053.
