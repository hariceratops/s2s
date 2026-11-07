# Struct views: follow-ups from the review

Left over from the review of PR #22 (issues 074 to 082). None blocks anything;
each is small.

## Design notes the architect never wrote
`dev/design/reading-two-fields-costs-a-whole-record.md` is silent on two
decisions made during implementation:
- A constraint on a nested record or an array of records takes the decoded
  record as `const Nested&` (078).
- `test/must_not_compile/struct_view_misuse.cpp` has a case 11, a `variance`
  inside an `array_of_records` element, that §7.3's table does not list. It is
  there to meet 080's array-element criterion for `variance`.

## Findings from rule-check, not fixed
- `docs/reference.md` shows `operator[]` as `-> const view_read_t<...>`; the
  header has `-> view_read_t<...>` with the `const` inside the alias.
- `docs/views.md` still repeats the creation semantics that `docs/reference.md`
  and `docs/errors.md` state, and the UBSan command appears in the README, the
  guide and `AGENTS.md`. AGENTS.md says nothing is documented twice.
- `test/schema/struct_view_allocation.cpp:97` uses `asm volatile`, which MSVC
  rejects. AGENTS.md lists MSVC as supported.
- Overlong lines left by a comment reflow: `AGENTS.md` around lines 18, 83 and
  84, and `test/must_not_compile/CMakeLists.txt` around line 22.
- Slice numbers remain in comments in `test/internals/fixed_layout_schema_ct.cpp`
  and `test/schema/struct_view_read_ct.cpp`, as elsewhere in the repo. No rule
  forbids it.
- The README says the clang UBSan build stops at
  `test/schema/delimited_write_ct.cpp`; that comes from 074.
