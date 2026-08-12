# [test] Optional fields move to test/fields/ and gain compile-time parity

Covers `maybe<field, present_only_if>`, whose presence is a predicate over
sibling fields rather than a stored flag. Same shape as 029.

Sources today: `test/runtime/optional_fields.cpp` (8 cases),
`test/runtime/write_optional_fields.cpp` (13 cases), and the corresponding
portions of the `constexpr/` files.

Depends on: 028.

## Acceptance Criteria
- Tests live at `test/fields/optional_{read,write}.cpp` (GoogleTest) and
  `test/fields/optional_{read,write}_ct.cpp` (ut, dual-mode).
- Routing is by constant-evaluability, not by whether a file is opened: a
  body the compiler can evaluate on its own goes to ut, everything else
  (both `ifstream` and `stringstream` bodies) stays GoogleTest. Nothing is
  in both.
- Per-feature GoogleTest suite name; no catch-all suite names survive the move.
- Existing `static_assert`s port 1:1, adding no coverage.
- Pass/fail results identical before and after the move.
- `maybe` has at least one **read**-side compile-time case.
- Compile-time coverage includes both predicate outcomes — present and absent —
  and a wrapped field that is not a trivial (the existing runtime suite covers
  optionals over arrays, strings, vectors and records; the compile-time tier
  covers at least one non-trivial wrapped kind).
- `always_present` is covered at compile time.
- Old sources deleted; `ctest` green.

## Notes 2026-08-12

Done. `ctest` 84/84 (was 78; +8 entries, −2).

GoogleTest inventory unchanged at 21 cases: `S2STest` (8) → `OptionalRead`,
`WriteOptionalFields` (13) → `OptionalWrite`. The read case names were
snake_case predicate descriptions (`optional_fixed_string_with_successful_parse_predicate`)
and became behaviour sentences.

Ported to ut and removed from `constexpr_write.cpp`: 13 `static_assert`s
covering presence, absence, both misuse directions, and the three conditional-
length cases. From `constexpr_read.cpp`: the 7 `opt_field_struct` asserts.

### The port caught a coverage change before it landed

Writing the conditional-length rejection from the runtime suite's wording, I
used `error_reason::validation_failure`. The `static_assert` being replaced
says `found_contradicting_length`. A disagreeing length is its own error
reason, not a generic validation failure, and porting it as the latter would
have silently widened what the test accepts. Checked against the original
rather than against the neighbouring magic-field tests, which do use
`validation_failure`.

`optional_read_ct.cpp` is 7 tests, 27 asserts. Beyond the 1:1 port: the absent
case with a trailing field behind the optional — an unsatisfied predicate has
to leave the payload's bytes *unread* rather than read and discard them, and a
field behind it is the only way to see which happened; a wrapped record, which
recurses into its own leaves where a trivial payload does not; the declared
byte order applied to a present payload; `always_present` holding regardless of
what the siblings say; and a present payload with no bytes left, which fails
rather than quietly coming back absent.

### Note for 035

Slicing `constexpr_write.cpp` on the optional section cut `alt_1`/`alt_2` out
with it — they sit between the optional helpers and `union_struct` but belong
to the union tests. Restored in place. The remaining sections are interleaved
this way too; check the boundary rather than trusting the section order.
