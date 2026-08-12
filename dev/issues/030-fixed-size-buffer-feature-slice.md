# [test] Fixed-size buffers move to test/fields/ and gain compile-time parity

Covers the four const-sized descriptors: `fixed_array_field`, `c_arr_field`,
`fixed_string_field`, `c_str_field`. Same shape as 029.

Sources today: `test/runtime/fixed_buffer_fields.cpp`,
`test/runtime/write_fixed_buffer_fields.cpp`, and the corresponding portions of
the `constexpr/` files.

Depends on: 028.

## Acceptance Criteria
- Tests live at `test/fields/fixed_buffer_{read,write}.cpp` (GoogleTest) and
  `test/fields/fixed_buffer_{read,write}_ct.cpp` (ut, dual-mode).
- Routing is by constant-evaluability, not by whether a file is opened: a
  body the compiler can evaluate on its own goes to ut, everything else
  (both `ifstream` and `stringstream` bodies) stays GoogleTest. Nothing is
  in both.
- Per-feature GoogleTest suite name; no catch-all suite names survive the move.
- Existing `static_assert`s port 1:1, adding no coverage.
- Pass/fail results identical before and after the move.
- Each of `fixed_array_field`, `c_arr_field`, `fixed_string_field`, and
  `c_str_field` has at least one **read**-side compile-time case.
- Compile-time coverage includes per-element byteswapping (not per-buffer) and
  the nested-aggregate case, since those are the behaviours the runtime suite
  already treats as load-bearing.
- Compile-time coverage includes the terminator accounting for the string
  descriptors (`N + 1` bytes consumed).
- Old sources deleted; `ctest` green.

## Notes 2026-08-12

Done. `ctest` 60/60 (was 54; +8 entries, −2).

GoogleTest inventory unchanged at 9 cases, relocated and renamed:
`MetaStructReadTest` (4) → `FixedBufferRead`, `WriteFixedBufferFields` (5) →
`FixedBufferWrite`. The read-side case names lost their `FromBinaryFile`
suffix, which described the fixture rather than the behaviour.

Ported to ut and removed from `constexpr_write.cpp`: the three aggregate
`static_assert`s (`roundtrip_aggregate<false>`, `<true>`,
`foreign_bytes_are_element_swapped`). The read side had **no** fixed-buffer
`static_assert`s at all, so `fixed_buffer_read_ct.cpp` is entirely new: 7
tests, 37 asserts, all 7 running at compile time.

Read-side compile-time coverage: all four descriptors read in one schema
(which also pins their relative offsets), per-element byteswap against known
foreign bytes, the swap descending into a nested aggregate, terminator
accounting for both string descriptors via a trailing field, exhaustion one
byte short of a terminator, and `c_arr_field` in both byte orders.

### A library defect fell out of this

The `c_arr_field`/`c_str_field` read requirement did not compile. `bit_cast`
cannot return a C array, so the constexpr read path never supported either
descriptor — while `docs/compile-time.md` lists both as "Fine at compile time".
Filed and fixed as 042; it blocked this issue's done-bar and the claim was
already in the docs. The write direction was always fine, which is why the
existing constexpr suite missed it.

### Convention note for 031-037

`expect(eq(...))` on a `std::array` does not compile: ut prints the failing
value and `std::array` has no `operator<<`. Compare element by element, or
loop over a `constexpr` expected table declared inside the test lambda — a
local is fine, a *capture* is what ut silently skips.
