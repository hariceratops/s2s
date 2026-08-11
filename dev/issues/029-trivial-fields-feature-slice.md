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
