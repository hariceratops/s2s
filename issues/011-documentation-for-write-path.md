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

## Notes 2026-08-07
- Every complete code sample was verified by compiling and running it, not by
  inspection. Doing so caught a wrong claim in the first draft: the guide
  implied that reading a derived field gives the derived value. It does not —
  derivation happens during the write, against a `const` struct, and is never
  written back, so the subscript returns the stored slot. Documented
  explicitly, with the `obj["count"_f] == 0` case spelled out.
- The worked example and the derived-field claims now live in
  `test/single_header/doc_example.cpp`, compiled and run by ctest, so the docs
  cannot drift from the library unnoticed.
- `UserGuide.md`'s Errors section documented `enum cast_error { ... }`, which
  never existed: the enum is `error_reason` and `cast_error` is the struct
  pairing it with `failed_at`. Corrected while adding the fourth enumerator.
