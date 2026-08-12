# [test] Magic fields move to test/fields/ and gain compile-time parity

Covers `magic_byte_array`, `magic_string`, `magic_number` — the descriptors
that check their value as part of reading. Same shape as 029.

Sources today: `test/runtime/magic_fields.cpp`,
`test/runtime/write_magic_fields.cpp`, and the corresponding portions of the
`constexpr/` files.

Depends on: 028.

## Acceptance Criteria
- Tests live at `test/fields/magic_{read,write}.cpp` (GoogleTest) and
  `test/fields/magic_{read,write}_ct.cpp` (ut, dual-mode).
- Routing is by constant-evaluability, not by whether a file is opened: a
  body the compiler can evaluate on its own goes to ut, everything else
  (both `ifstream` and `stringstream` bodies) stays GoogleTest. Nothing is
  in both.
- Per-feature GoogleTest suite name; no catch-all suite names survive the move.
- Existing `static_assert`s port 1:1, adding no coverage.
- Pass/fail results identical before and after the move.
- `magic_byte_array`, `magic_string`, and `magic_number` each have at least one
  **read**-side compile-time case.
- Compile-time coverage includes the rejection path for each of the three, not
  only the matching path — a magic field that never fails is not tested.
- A rejected magic value produces a compile-time-observable `cast_error`
  naming the offending field.
- Old sources deleted; `ctest` green.

## Notes 2026-08-12

Done. `ctest` 78/78 (was 71; +9 entries, −2).

GoogleTest inventory unchanged at 11 cases: `S2STest` (4) → `MagicRead` (4),
`WriteMagicFields` (7) → `MagicWrite` (6) + `ConstraintWrite` (1). The moved
case is `RejectsConstraintViolationOnAnOrdinaryField`, whose own comment
already said constraints are not a magic-field feature; it seeds
`test/fields/constraint_write.cpp` for 036, following 029's `record_read`
precedent.

Ported to ut and removed from `constexpr_write.cpp`: 6 `static_assert`s
(`roundtrip_magic` in both byte orders and the four on `write_wrong_magic`).
The four rejection asserts collapse into one ut test rather than four, since
they interrogate a single call's error.

The read side had **no** magic `static_assert`s, so `magic_read_ct.cpp` is
entirely new: 7 tests, 22 asserts. All three descriptors matching and all
three rejecting, each naming its field. Every schema carries a trailing
`payload` field, because a magic field that rejects has to stop the read and a
field behind it is the only way to see that it did. The `magic_byte_array`
rejection differs in its last byte, so a comparison that stops early would
pass it.

Also covered: a magic number matched in big endian. The magic value is a byte
pattern rather than a number, so the same bytes read big endian still have to
match rather than compare against a swapped value.
