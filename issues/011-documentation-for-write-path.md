# [chore] Document the write path

Ship user-facing documentation with the feature rather than as a follow-up.
This includes calling out the read-only derived-field change, which breaks
existing read-side code that assigns to a length field.

Depends on: 001–010.

## Acceptance Criteria
- `UserGuide.md` gains a "Writing to stream" section mirroring the structure
  of the existing read section, with a worked example that populates a struct
  and writes it in both byte orders.
- That section documents which fields are derived and read-only
  (`len_from_field` targets, `type_switch` discriminants), why, and that
  assigning to them is a compile error.
- The breaking change is called out explicitly: code that previously did
  `s["len"] = 5` on a parsed struct no longer compiles, and what to do instead.
- The section documents which inconsistencies are caught at write time and
  what error each produces (`parse_if` presence mismatch, `len_from_fields`
  disagreement, fan-out contradiction, `if_else_ladder` mismatch, constraint
  violation, length-width overflow).
- The write-once/fail-fast contract is stated: a failed write may leave a
  partially written stream and the caller handles it.
- `README.md`: the "(The read in other direction is work-in-progress)" note is
  removed or updated, the feature list mentions write support, and the roadmap
  item is checked off.
- Any code sample in the docs compiles as written.
