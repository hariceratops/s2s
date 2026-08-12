# [test] Retire test/runtime/ and test/constexpr/, audit the coverage bar

Closing slice. The feature slices each move their own tests; this one confirms
nothing was left behind and verifies the spec's hard done-bar — **no schema
construct listed in `docs/schema/` lacks at least one compile-time case.**

This is the only place the bar is checked as a whole. Each feature slice
asserts it for its own descriptors; a construct owned by no slice would
otherwise slip through unnoticed.

Depends on: 029-038. Independent of 039 and 040.

## Acceptance Criteria
- `test/runtime/` and `test/constexpr/` no longer exist, and `test/CMakeLists.txt`
  no longer references them.
- Every row of the descriptor table in `docs/schema/index.md` is walked
  against the ut suites, and each row is shown to have at least one
  compile-time case. The mapping — row to suite — is recorded in a `## Notes`
  section so the next person can re-check it without rederiving it.
- Every row of the form table in `docs/schema/size-axis.md` is walked the same
  way.
- Any row found uncovered is either covered here or recorded as a deliberate
  exception with a reason; "we ran out of time" is not a reason.
- No test source anywhere is unregistered in CMake.
- No GoogleTest binary links `ut`, and no `ut` binary links GoogleTest.
- The claims in `docs/compile-time.md` are re-read against what the suites
  actually cover, and corrected if the page overstates or understates.
- `ctest` green on a clean configure.

## Notes 2026-08-12

Done. `ctest` 106/106 on a clean configure (`rm -rf build`, gcc 14.2,
CMake 3.28.3).

`test/runtime/` was retired in 037 and `test/constexpr/` in 038, so this slice
audits rather than moves. `test/` now holds `fields/`, `internals/`,
`rejected_misuse/`, `as_shipped/`, `utils/`, plus `CMakeLists.txt` and
`ct_coverage_check.cmake`. Nothing in `test/CMakeLists.txt` refers to the old
directories.

### The compile-time tier, in totals

22 ut sources, 134 tests, **134 of them running at compile time**, 573
asserts. Each source's `_coverage` entry enforces the middle number, so
"134 of 134" is checked on every `ctest` run rather than counted once here.

For scale: before this feature the read side had two `u32` fields and one
buffer-exhaustion case.

### Descriptor table walk (`docs/schema/index.md`)

Every row, against the suite holding its **read**-side compile-time case.
Write-side suites exist for all of them too and are not listed.

| Descriptor | Read-side compile-time case |
|---|---|
| `basic_field` | `trivial_read_ct.cpp` |
| `fixed_array_field` | `fixed_buffer_read_ct.cpp` |
| `c_arr_field` | `fixed_buffer_read_ct.cpp` |
| `array_of_records` | `record_read_ct.cpp` |
| `fixed_string_field` | `fixed_buffer_read_ct.cpp` |
| `c_str_field` | `fixed_buffer_read_ct.cpp` |
| `str_field` | `variable_buffer_read_ct.cpp` |
| `vec_field` | `variable_buffer_read_ct.cpp` |
| `vector_of_records` | `record_read_ct.cpp` |
| `struct_field` | `record_read_ct.cpp` |
| `magic_byte_array` | `magic_read_ct.cpp` |
| `magic_string` | `magic_read_ct.cpp` |
| `magic_number` | `magic_read_ct.cpp` |
| `maybe` | `optional_read_ct.cpp` |
| `variance` | `union_read_ct.cpp` |

15 of 15. No exceptions.

### Size-axis form table walk (`docs/schema/size-axis.md`)

| Form | Read-side compile-time case |
|---|---|
| `fixed<N>` | every read suite |
| `len_from_field<"id">` | `variable_buffer_read_ct.cpp` |
| `size_from_fields` / `len_from_fields` | `size_axis_read_ct.cpp` (the two are aliases, `field_size.hpp:43`) |
| `size_choices<...>` | **none — deliberate exception** |
| `size_dont_care` | `record_read_ct.cpp`, `size_axis_read_ct.cpp` (never written by hand; reached through `struct_field` and `array_of_records`) |

### The two deliberate exceptions

Both are constructs that **cannot currently be used**, not constructs that were
skipped. Each was re-probed during its slice rather than taken on trust from
the issue that reported it.

1. **`size_choices`** — 027. `is_selectable_size_v` reads `is_fixed_size<T>::res`,
   so `selectable_size_like` rejects the one type it exists for. A `basic_field`
   declared with `field_size<size_choices<...>>` fails to compile (3 errors,
   gcc 14.2). Covered the moment 027 lands.
2. **Range constraints** (`range`, `is_in_open_range`, `is_in_closed_range`) —
   026, and already documented as unusable in `docs/constraints.md`. `range`'s
   constructor `static_assert`s on its own arguments, so any use is a hard
   compile error.

### Framework separation, checked in the build tree

No target both links GoogleTest and sees ut's include path. Measured from the
generated `link.txt` and `flags.make` on a clean build, not by reading CMake:
19 targets link gtest, 66 see ut (22 sources x 3 builds), **0 do both**.

A first pass at this check reported every target as "both". The check was
wrong — a `grep -c ... || echo 0` that never yielded 0 — and it was reading a
build tree still holding target directories from before the reorg
(`magic_fields`, `vec_of_recs_fields`, …). Rerun on a wiped tree with the
comparison fixed. Worth recording: a stale build directory makes an audit of
the build directory lie.

### No unregistered test source

Every `.cpp` under `test/` and `examples/` is named in its directory's
`CMakeLists.txt`. This is what let `static_tests_1.cpp` rot for so long — it
was unregistered, so nothing noticed it could not compile.

### `docs/compile-time.md` understated the second claim

The page said a compile-time parse requires "no allocating fields", with a
table listing `str_field`, `vec_field`, `vector_of_records` and a `variance`
over `as_string`/`as_vec` alternatives as disqualifying. All four parse at
compile time and are covered by tests — 031 and 032 found the first three, and
the union case was probed here specifically (0 errors).

The real boundary is narrower: an allocation made and released inside one
constant evaluation is transient and allowed; what fails is a **result that
still owns heap memory when the evaluation ends**. Probed both sides —
`constexpr auto res = parse();` holding a `std::string` gives "the value of
'res' is not usable in a constant expression", while the same parse inside a
function returning `bool` compiles.

Rewritten to say that, with both forms shown. The fixed-size descriptors keep
their purpose in the corrected text — they are the ones whose results can
escape into a namespace-scope `constexpr` — which is a sharper reason for them
existing than "the others do not work". `doc_examples_match` and
`docs_nav_lists_every_page` still pass; the two new blocks are fragments, so
neither needs a binding.
