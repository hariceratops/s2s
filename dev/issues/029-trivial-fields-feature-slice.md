# [test] Trivial fields move to test/fields/ and gain compile-time parity

First feature slice, and the template for 030-037. `basic_field` coverage moves
out of the execution-mode directories into `test/fields/`, its pure assertions
become a ut suite, and the read-side compile-time gap for trivials is filled.

Sources today: `test/runtime/trivial_fields.cpp`,
`test/runtime/write_trivial_fields.cpp`, and the trivial-field portions of
`test/constexpr/constexpr_read.cpp` and `constexpr_write.cpp`.

Depends on: 028.

## Acceptance Criteria
- Tests live at `test/fields/trivial_read.cpp`, `test/fields/trivial_write.cpp`
  (GoogleTest, `ifstream`) and `test/fields/trivial_read_ct.cpp`,
  `test/fields/trivial_write_ct.cpp` (ut, dual-mode).
- Routing is by constant-evaluability, not by whether a file is opened: a body
  the compiler can evaluate on its own goes to ut, everything else stays
  GoogleTest. Note that the write-side runtime tests use `std::stringstream` —
  no file, but not constant-evaluable either, so they stay GoogleTest. Nothing
  is covered by both; a case ported to ut is deleted from GoogleTest, not
  duplicated.
- As the first slice, this one fixes the per-feature conventions the rest
  follow: source names, target names, CTest test names, and suite naming. If
  the design's convention does not survive contact here, it is corrected in
  the design before 030 starts, not diverged from silently.
- GoogleTest cases carry a per-feature suite name; `S2STest` and the other
  catch-all suite names no longer appear in the moved cases. Case names follow
  the existing behaviour-sentence style (`ByteswapsPerElementNotPerBuffer`).
- Existing `static_assert`s for trivials port to ut suites 1:1, adding no
  coverage.
- The full set of pass/fail results for trivial fields is identical before and
  after the move. Any difference is a porting bug and is fixed before this
  issue closes.
- `basic_field` has at least one compile-time case on the **read** side
  covering, at minimum: value round-trip, declared endianness, and buffer
  exhaustion.
- The old trivial-field sources are deleted, not left behind.
- `ctest` is green.

## Notes 2026-08-12

Done. `ctest` 51/51 on a clean configure (was 46; +7 entries, −2).

Case inventory is unchanged at 8 GoogleTest cases, relocated and renamed:
`MetaStructTest` (4) + `WriteTrivialFields` (4) → `TrivialRead` (3) +
`RecordRead` (1) + `TrivialWrite` (4).

`test/runtime/trivial_fields.cpp` held `NestedStructReadingFromBinaryFile`,
which is a `struct_field` case, not a trivial one. Deleting the file meant
relocating it, so `test/fields/record_read.cpp` exists early with that single
case; 032 fills it out rather than creating it. The alternative — leaving a
one-case file named after trivials — was worse.

Ported to ut and removed from the shared constexpr sources: 6 `static_assert`s
from `constexpr_read.cpp`, 5 from `constexpr_write.cpp`. `constexpr_read.cpp`'s
`main` also lost the `test_001`/`test_002` debug printers and a `sizeof`
`std::cout` line, none of which asserted anything.

New read-side compile-time coverage beyond the 1:1 port: big-endian reads
(the same bytes must give byte-reversed values, or the endianness axis is not
being applied), and a violated constraint naming the offending field.

**Verified that the compile-time tests actually run at compile time.** Built
each ut source with neither `UT_COMPILE_TIME_ONLY` nor `UT_RUN_TIME_ONLY`, so
ut reports both counts: `4 passed, 0 failed, 4 compile-time` for each. This
matters because ut skips a *capturing* test lambda at compile time silently —
see the harness gap below.

### Harness gap found here, not yet fixed

A `_compile_time` CTest entry proves the translation unit compiles. It does
**not** prove any test ran at compile time: a capturing lambda is skipped with
no diagnostic, and the entry stays green. Today the only way to tell is the
manual no-macro build above. That is a hole in 028's helper, and it will get
wider with every slice that adds a ut source. Options, cheapest first: have
`add_ut_test` build a third no-macro target and fail if its output reports
`0 compile-time`; or replace the `UT_RUN_TIME_ONLY` target with a no-macro one
and check the same thing. Not done here — it changes the shared helper, which
is 028's territory.
