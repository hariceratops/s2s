# [chore] Schema: the four axes and the descriptor table

Opens the `UserGuide.md` Schema section — the direction-neutral spine that both
`struct_cast` and `struct_write` are driven by. Field descriptors are the schema
language, not a read-side feature; organising the docs by direction is what
produced the current asymmetry.

## Acceptance Criteria
- A "Schema" section exists in `UserGuide.md` introducing the four axes every
  descriptor declares: `id`, `type`, `size`, `constraint`.
- It is stated explicitly that the same declaration drives both directions, so
  nothing on these axes is documented twice per direction.
- A descriptor table covers every alias exported from
  `include/api/field_descriptors.hpp`, including the three currently absent from
  all prose in both files: `struct_field`, `c_arr_field`, `c_str_field`.
- `fixed_string` and `as_fixed_string` are documented as supported public
  surface, with the reason they are unavoidable: `magic_string` and
  `fixed_string_field` use `fixed_string` as the field type, so assigning to a
  magic field means constructing one.
- A documentation-scope statement names `static_vector`, `static_map`, and the
  `include/lib/` metaprogramming helpers as internal and not covered — stated as
  a scope decision, not silently omitted.
- Any non-trivial example is backed by a registered CTest target under
  `test/single_header/`.
