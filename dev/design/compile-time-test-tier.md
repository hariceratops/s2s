# Design: A real compile-time test tier, on qlibs/ut

Consumes `dev/specs/compile-time-test-tier.md` and `dev/issues/028`–`dev/issues/041`.

Every mechanical claim below about qlibs/ut was verified by building it, not
read off the README: header `https://raw.githubusercontent.com/qlibs/ut/v2.1.6/ut`,
compiler `g++-14` (Ubuntu 14.x), `-std=c++23`, against this repo's
`include/s2s.hpp` and `test/utils/constexpr_memstream.hpp`. Where a spec or
issue statement did not survive that check, §10 says so plainly rather than
designing around it quietly.

Settled upstream and not reopened here: ut is adopted, fetched by `URL` +
`URL_HASH SHA256` + `DOWNLOAD_NO_EXTRACT TRUE`; GoogleTest and ut never share
a binary; the unit of "a feature's coverage reads in one place" is the
directory; the migration changes organisation, not coverage.

---

## 1. What ut actually does — verified behaviour

| Claim | Verdict | Evidence |
|---|---|---|
| `UT_COMPILE_TIME_ONLY` removes the run-time execution block **and** the reporter destructor (`#ifndef UT_COMPILE_TIME_ONLY ~reporter()`) | true | header ll. 785–793, 819–823 of v2.1.6; built binary prints nothing, exits 0 |
| A compile-time-only build still needs `main` | true | tests are statements inside a function; the runner template is only instantiated from there, and an executable still has to link. No `#ifdef` in the source: the same `int main()` serves both builds |
| The run-time-only build loses the compile-time assertions entirely | true (this is the point) | summary reports `0 compile-time`; the `static_assert((run.test(), …))` block is inside `#ifndef UT_RUN_TIME_ONLY` |
| A run-time failure aborts | true | `~reporter()` calls `__builtin_abort()`; observed exit 134, CTest reports `Subprocess aborted` |
| A compile-time failure names the test and the failing line | true | see the captured diagnostic below |
| A body that cannot be constant-evaluated is a **compile error** in the compile-time build, and fine in the run-time build | true | an `ifstream` body gives `error: call to non-'constexpr' function … basic_ostream::write` under `UT_COMPILE_TIME_ONLY`, and passes under `UT_RUN_TIME_ONLY` |
| A **capturing** test lambda is silently skipped at compile time | true, and dangerous | `runner::on` guards the `static_assert` block with `!has_capture_lambda_v<Test>` (`sizeof(Fn) > 1`). A captured variable turns a compile-time test into a run-time-only test with no diagnostic |
| `mutable` / `UT_RUN_TIME` marks a test run-time-only | true | same guard |
| `expect(some_bool)` does not compile | true | `expect` static_asserts "Expression required"; use `expect(eq(x, true))` |
| Compared values must be streamable to `std::clog` in the non-compile-time-only builds | true | `detail::stream::operator<<` static_asserts `requires { std::clog << t; }`. `s2s::error_reason` is an **unscoped** enum and `cast_error::failed_at` is a `string_view`, so both are fine; verified `expect(eq(err.failure_reason, s2s::buffer_exhaustion))` and `expect(eq(err.failed_at, std::string_view{"b"}))` |
| `-DNTEST` (skip ut's self-verification on include) is worth defining | **no** | measured 4.01 s vs 4.22 s on a real s2s suite. Keep the self-check |
| main and tag `v2.1.6` differ | only in the README block; the code below `#pragma once` is byte-identical | `diff` of both from `#pragma once` → 0 lines |

Captured compile-time diagnostic (gcc 14, expectation deliberately broken —
this is the evidence issue 028's `## Notes` asks for, so it need not be
re-derived):

```
utinc/ut:814: error: non-constant condition for static assertion
  static_assert((run.test(), R"([FAILED] Compile-time expectation failed. …)"));
  required from 'constexpr ut::detail::test<Name>::run::run(...)
    [with fixed_string Name = fixed_string<34>{"little endian trivials round trip"}]'
pilot_bad.cpp:28:11:   in 'constexpr' expansion of 'ut::expect.operator()<ut::eq<unsigned int, unsigned int>>(
    ut::eq<unsigned int, unsigned int>((*&(& res.std::expected<s2s::struct_field_list_impl<2,
    s2s::field<s2s::fixed_string<1>{std::array<char, 2>{"a"}}, unsigned int, … >>::operator*())
    ->operator[]<s2s::field_accessor<…{"b"}>>(…)), 3405697038), …)'
utinc/ut:889: error: call to non-'constexpr' function 'void ut::detail::failed()'
```

Answer to the spec's open question: **the diagnostic is usable, not clean.**
It carries the test name, the exact source line of the failing `expect`, and
the expected value (`3405697038`); it also carries one line of s2s
`struct_field_list` template soup because the receiver type is printed. That
is strictly more than a bare `static_assert`, which gives a line and nothing
else. The run-time build is where the actual value shows up:
`pilot_bad.cpp:28:FAILED:"little endian trivials round trip": 3405697037 == 3405697038`.
Both of ut's two remaining advantages therefore hold on this codebase. No
grounds to revisit the adoption.

### 1.1 Name collisions with the s2s vocabulary

ut exports `eq, neq, lt, gt, le, ge, nt` and the literal `operator""_f`
(float). s2s exports the constraints `eq, neq, lt, gt, lte, gte` and the field
accessor literal `"a"_f`. The literals do not collide (one is a numeric
literal operator, the other a string one — verified: `"a"_f` resolves to s2s's
even with `using namespace ut;`), but the four constraint names do.

**Rule for every ut source: no `using namespace ut;`.** Import exactly
`using ut::expect; using ut::eq; using ut::operator""_test;` and qualify s2s
constraints as `s2s::eq{…}`. Verified end to end on a constraint suite that
uses both in the same expectation.

---

## 2. The CMake mechanism

### 2.1 Where things live

`test/CMakeLists.txt` becomes the one place that declares dependencies and
test-kind helpers; the per-directory `CMakeLists.txt` files only *call* them.
Today `runtime/CMakeLists.txt` owns the GoogleTest `FetchContent` declaration
and `add_struct_cast_test`, which is exactly what blocks the migration: the
new `test/fields/` needs both while `test/runtime/` is being emptied, and
`test/runtime/` is deleted in 041.

```cmake
# test/CMakeLists.txt
include(FetchContent)

FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0)
FetchContent_MakeAvailable(googletest)

# The header ships as a file literally named `ut`, with no extension, so it is
# downloaded and not extracted. Pinned by tag *and* hash: the tag says which
# release, the hash says the tag did not move.
FetchContent_Declare(qlibs_ut
  URL https://raw.githubusercontent.com/qlibs/ut/v2.1.6/ut
  URL_HASH SHA256=80afe19304e2401df66f842a3698a87a56d694572014a0e32a51dc854e58ac01
  DOWNLOAD_NO_EXTRACT TRUE)
FetchContent_MakeAvailable(qlibs_ut)   # no CMakeLists.txt there; MakeAvailable
                                       # skips add_subdirectory, by design

add_library(qlibs_ut INTERFACE)
target_include_directories(qlibs_ut SYSTEM INTERFACE ${qlibs_ut_SOURCE_DIR})
```

Verified end to end with CMake 3.28: the file lands at
`_deps/qlibs_ut-src/ut`, `#include <ut>` resolves, and `FetchContent_MakeAvailable`
does not choke on the missing `CMakeLists.txt`. `SYSTEM` keeps ut's own
internals out of `-Wall -Wextra -Wpedantic`, which the root `CMakeLists.txt`
applies globally; the target is named `qlibs_ut` rather than `ut` so it cannot
be confused with boost-ext/ut, which is a different library.

### 2.2 `add_ut_test`

```cmake
# One source, two builds. ut's compile-time mode is `static_assert` inside the
# TU, so a compile-time failure is a build failure; its run-time mode aborts
# from a static destructor. Two entries keep the two apart.
function(add_ut_test name)
  add_executable(${name}_compile_time ${name}.cpp)
  target_compile_definitions(${name}_compile_time PRIVATE UT_COMPILE_TIME_ONLY)
  target_link_libraries(${name}_compile_time PRIVATE qlibs_ut)

  add_executable(${name}_run_time ${name}.cpp)
  target_compile_definitions(${name}_run_time PRIVATE UT_RUN_TIME_ONLY)
  target_link_libraries(${name}_run_time PRIVATE qlibs_ut)

  # The compile-time binary does nothing when run — the assertions were spent
  # during compilation. Running it would report a stale binary as a pass, so
  # the entry drives the build instead, the way compile_fail/ already does.
  add_test(NAME ${name}_compile_time
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
            --target ${name}_compile_time --config $<CONFIG>)
  set_tests_properties(${name}_compile_time PROPERTIES RESOURCE_LOCK build_tree)

  add_test(NAME ${name}_run_time COMMAND ${name}_run_time)
endfunction()
```

Neither target links GoogleTest, and no GoogleTest target gets `qlibs_ut` —
the separation the spec demands is a property of this function, not of
reviewer discipline.

**Why the compile-time entry drives a build instead of running the binary.**
Measured, not assumed. With `add_test(COMMAND ${name}_compile_time)`:

- broken source, no binary ever built → CTest says `(Not Run)`, a correct fail;
- broken source, binary left over from an earlier good build → **CTest reports
  a pass.** Reproduced: `cmake --build` fails, `ctest` then prints
  `100% tests passed` for the compile-time entry.

A false green on the entry whose only job is to say "the compile-time tier is
intact" is not acceptable. Driving the build costs 0.14 s for the whole
`ctest` run when everything is up to date, captures the compiler diagnostic
into `LastTest.log`, and reproduces the failure correctly in both scenarios
above. `RESOURCE_LOCK build_tree` serialises these entries so `ctest -j` never
runs two `cmake --build` invocations against one build tree; the existing four
`compile_fail/` entries should take the same lock when they move (§3.2), for
the same reason.

The targets stay in `ALL`. A plain `cmake --build` therefore compiles them in
parallel and fails loudly on a broken compile-time test — which is honest,
because it *is* a compile error — and the CTest entry is the same check
re-asked cheaply, per feature, with a name attached.

### 2.3 Which s2s header a ut source compiles against

`include/s2s.hpp`, not `single_header/s2s.hpp`. The entire reason for adopting
ut is diagnostics; a compile-time failure whose file/line points into a
20k-line generated artifact costs the one advantage being bought. The runtime
GoogleTest feature sources keep including the single header (through
`s2s_test_utils.hpp`) exactly as they do today, and `test/as_shipped/`
(§3.2) compiles the artifact deliberately, so the artifact stays covered.
This preserves the status quo split — the old `constexpr/` sources already
include `include/s2s.hpp` — rather than introducing a new one.

One repo defect to fix while doing it: `test/utils/constexpr_memstream.hpp`
includes `../../single_header/s2s.hpp`, so today's `constexpr/*.cpp` pull in
*both* headers and get the source-tree definitions only because they include
`include/s2s.hpp` first and the amalgam preserves the include guards. That is
accidental. The helper needs only `s2s::constexpr_stream` from
`include/stream/stream_traits.hpp`; point it there (§4).

### 2.4 Fixture collision under `ctest -j`

Pre-existing, not introduced here, but the reorg concentrates it and this
design's `RESOURCE_LOCK` note makes `-j` more attractive: 18 of the 21
file-touching GoogleTest cases write **the same** `test_input.bin` into the
shared `CMAKE_CURRENT_BINARY_DIR`. Parallel CTest is unsafe today. The cheap
fix belongs in the hoisted helper, once, rather than in nine slices:

```cmake
function(add_struct_cast_test name)
  add_executable(${name} ${name}.cpp)
  add_dependencies(${name} generate_single_header)
  target_link_libraries(${name} PRIVATE gtest gtest_main)
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${name}.dir)
  add_test(NAME ${name} COMMAND ${name}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${name}.dir)
endfunction()
```

This changes where a scratch file lands, not what any test asserts.

---

## 3. Names

### 3.1 Directories

| Now | Becomes | Why |
|---|---|---|
| `test/runtime/`, `test/constexpr/` | `test/fields/` | per the spec: one directory, one entry per schema construct |
| — (new) | `test/internals/` | machinery that is not a schema construct: traits and concepts, `include/lib/` containers, field-list metadata, `meta::type_id` |
| `test/compile_fail/` | `test/rejected_misuse/` | states the claim (misuse is rejected) instead of the mechanism (compilation fails). Holds the positive control too, which is the boundary of the same claim |
| `test/single_header/` | `test/as_shipped/` | one name for both halves of what that directory verifies: the amalgamated artifact a consumer actually includes, and every example the documentation shows them |
| `test/hello/` | `examples/` (repo root) | per issue 040 |
| `test/utils/` | unchanged | §4 |

**Superseded 2026-08-12, after the feature landed.** Three of these names did
not survive first contact with a reader. `test/fields/` collided with
`include/field/` and `include/field_list/`, which are different things, and
`fields` never named what the directory groups by — it is one entry per row of
the descriptor table in `docs/schema/index.md`, so `test/schema/` pairs with
the page it covers. `rejected_misuse/` was vague, and the "holds the positive
control too" reasoning above turned out to be the flaw rather than the
justification: a directory of programs that must not compile has no business
holding one that must, so that case moved to
`test/schema/size_axis_read_ct.cpp` and the directory became
`test/must_not_compile/`. `as_shipped/` tried to be one name for two jobs and
was 12 targets of documentation to 1 of packaging; split into
`test/doc_examples/` and `test/shipped_header/`, each name is exactly true.

| Design said | Now | Why the change |
|---|---|---|
| `test/fields/` | `test/schema/` | mirrors `docs/schema/`; no collision with `include/field*/` |
| `test/rejected_misuse/` | `test/must_not_compile/` | states the claim plainly, once the positive control moved out |
| `test/as_shipped/` | `test/doc_examples/` + `test/shipped_header/` | one name could not cover both jobs |

`test/internals/` exists so `test/fields/` can mean exactly "one entry per row
of the `docs/schema/` table" — which is the property issue 041's audit walks.
Mixing `containers_ct.cpp` in there would make that walk ambiguous forever.

Rejected names, so they are not re-proposed:

- `test/negative_compilation/`, `test/compile_errors/` — mechanism again, no gain over `compile_fail`.
- `test/shipped_artifact/` — accurate about `roundtrip.cpp` and the determinism check, but under-describes the eleven doc-example targets that are the bulk of the directory.
- `test/documented_examples/` — the mirror-image failure: describes the examples, not the artifact.
- `test/public_surface/`, `test/consumer/` — true of every test in the repo, so they say nothing.

Target names inside `as_shipped/` (`single_header_roundtrip`, `doc_example`, …)
are **not** renamed: issue 039 renames a directory, and CTest entry names are
what the developer types into `ctest -R`.

### 3.2 Files, targets and CTest entries

Per feature `<f>` and direction `<d>` ∈ {`read`, `write`}:

| Kind | Source | Target(s) | CTest entry(ies) |
|---|---|---|---|
| GoogleTest | `test/fields/<f>_<d>.cpp` | `<f>_<d>` | `<f>_<d>` |
| ut | `test/fields/<f>_<d>_ct.cpp` | `<f>_<d>_ct_compile_time`, `<f>_<d>_ct_run_time` | same two |

`_ct` names the *tier* (the pure tier, which runs in both modes), the trailing
word names the *build mode*. The redundancy buys something concrete:
`ctest -R _ct_compile_time` runs the whole compile-time tier and nothing else,
`ctest -R _ct_run_time` the whole thing at run time with values printed. That
selector is the reason to keep the suffixes rather than shorten to `_ct` /
`_ct_rt`.

GoogleTest suite name = PascalCase of the source stem: `trivial_read.cpp` →
`TEST(TrivialRead, …)`, `trivial_write.cpp` → `TEST(TrivialWrite, …)`. Case
names keep the write side's behaviour-sentence style
(`ByteswapsPerElementNotPerBuffer`). See §10 for how far this goes beyond what
the spec strictly authorises.

ut test names are prose sentences in lower case — they are strings, and they
are what the compile-time diagnostic prints:
`"a truncated buffer fails on the field that ran out"_test`.

`test/internals/` sources are ut-only and take a plain `_ct` stem:
`traits_ct.cpp`, `containers_ct.cpp`, `field_list_metadata_ct.cpp`.

---

## 4. `test/utils/` — stays, with one edit

Checked, since the brief flagged it as suspect: **`s2s_test_utils.hpp` does
not include `<gtest/gtest.h>`.** It includes `<fstream>`, `<sstream>` and the
single header, and three of its six macros
(`FIELD_LIST_LE_WRITE_REJECTED` and friends) *expand to* `ASSERT_FALSE` /
`EXPECT_EQ`. Macros are expanded at the use site, so including the header in a
ut TU is harmless — but pointless, because everything in it is about real
streams, which is precisely what does not belong in a ut source.

Decision: **no split.** The directory keeps its two headers and the division
of labour is by who includes what:

- GoogleTest feature sources include `s2s_test_utils.hpp` (and, through it,
  the single header) — unchanged from today.
- ut sources include `constexpr_memstream.hpp` and `include/s2s.hpp`, and
  nothing else from `utils/`.

Splitting a 87-line header whose only cross-framework content is four
`using u8 = …` aliases would be an abstraction with no forcing problem; the
aliases cost one line each to repeat, as the current `constexpr/` sources
already do.

Two edits are in scope:

1. `constexpr_memstream.hpp` includes `../../include/stream/stream_traits.hpp`
   instead of the single header (§2.3).
2. `s2s_test_utils.hpp`'s dead `field_list_le_read_check` template and the
   commented-out `_field_list_be_read_check` UDL go, once the last file that
   might have wanted them is rewritten (039/041, not the feature slices).

---

## 5. The routing rule is wrong as written — corrected

The spec says the boundary is mechanical: "touches a file" routes to
GoogleTest, everything else to ut. Applied literally to this repo it
mis-routes **59 of the 118 existing cases**. Measured:

| Group | Cases | Stream used |
|---|---|---|
| read side, `test/runtime/` | 37 of 44 | real `ifstream`/`ofstream` |
| read side, `field_validators.cpp` | 7 | none — constraint objects called directly |
| write side, `test/runtime/` | 59 | `std::stringstream`, **no file** |

`std::stringstream` is not a file, so the literal rule sends the entire write
suite to ut — where it cannot go. A `stringstream` body is not
constant-evaluable, so the compile-time target would refuse to compile it, and
those 59 cases are the *only* coverage of the runtime `writeable` overloads
that `AGENTS.md` requires alongside every `constexpr_stream` overload.
Rewriting them onto `memstream` would delete that coverage — the one thing the
spec forbids.

**Corrected rule, same mechanical character:** a case routes to ut iff its body
is evaluable in a constant expression — in practice, iff its only stream is
`memstream`. Anything holding a real stream object (`ifstream`, `ofstream`,
`stringstream`) stays in GoogleTest. This is enforced by the compiler, not by
review: mis-routing a stream-touching case into a ut source fails the
`_ct_compile_time` build with ut's own message telling you to mark it
`mutable`. (Verified: an `ifstream` body gives
`error: call to non-'constexpr' function … basic_ostream::write` under
`UT_COMPILE_TIME_ONLY` and passes under `UT_RUN_TIME_ONLY`.)

Marking such a test `mutable` to force run-time-only is available and is
**not** to be used: it produces a ut binary that duplicates what GoogleTest
already does better, with no compile-time entry behind it.

Consequence worth stating up front, because it resizes the whole job: under
the corrected rule the only GoogleTest cases that become ut cases are
`field_validators.cpp`'s seven. Everything else in the ut tier is either a
port of an existing `static_assert` (26 read + 46 write + 9 + 47 + 69 + 5) or
new read-side coverage. The "identical pass/fail before and after" gate is
therefore about file moves, suite renames and `static_assert`→`expect`
transcription — not about semantic re-homing of runtime tests.

### 5.1 Which feature owns a case that spans two

Needed because constraints (036) and the size axis (037) cut across every
descriptor, and because a case claimed by two slices gets moved twice, while a
case claimed by none disappears. Rule: **a case belongs to the feature whose
behaviour would have to break for the case to fail.**
`MetaStructTest.ValidationFailureOnFieldValue` lives in `trivial_fields.cpp`
today but fails only if constraint checking breaks — it moves to
`constraint_read.cpp`, not `trivial_read.cpp`. When genuinely ambiguous, the
narrower feature wins (a `maybe` wrapping a `vec_field` is an optional test).

### 5.2 A feature file with no cases is not created

`field_validators.cpp` is entirely pure, so after 036 the constraint feature
may own zero GoogleTest cases in one or both directions. Do not create an
empty `constraint_write.cpp`: an empty GoogleTest binary passes vacuously and
reads as coverage that does not exist. The directory-level promise is kept by
the `_ct` files. (Issue 036 lists all four files unconditionally; see §10.)

### 5.3 Porting recipe for the existing `static_assert`s

`constexpr_write.cpp` is already the right shape: `constexpr auto fn() -> bool`
helpers plus a trailing block of `static_assert(fn())`. Keep the helpers
verbatim, replace the block:

```cpp
"records round-trip in both byte orders"_test = [] {
  expect(eq(roundtrip_records<false>(), true));
  expect(eq(roundtrip_records<true>(), true));
};
```

Three rules that make the port behave:

- **No `constexpr` locals inside a test body.** `constexpr auto res = parse();`
  forces constant evaluation even in the run-time build, which silently turns
  that build's assertion into a tautology. Plain locals still evaluate at
  compile time in the compile-time build — the whole body sits inside
  `static_assert` (verified: summary still reports `1 compile-time`).
- **No captures.** A capture silently disables the compile-time run (§1).
- **No `using namespace ut;`** (§1.1).

---

## 6. Feature map, and the coverage bar

Every row of `docs/schema/index.md` maps to an owning slice; no row is
unowned, so 041's audit starts from a map rather than a blank page.

| Descriptor row | Feature stem | Slice |
|---|---|---|
| `basic_field` | `trivial` | 029 |
| `fixed_array_field`, `c_arr_field`, `fixed_string_field`, `c_str_field` | `fixed_buffer` | 030 |
| `str_field`, `vec_field` | `variable_buffer` | 031 |
| `array_of_records`, `vector_of_records`, `struct_field` | `record` | 032 |
| `magic_byte_array`, `magic_string`, `magic_number` | `magic` | 033 |
| `maybe` | `optional` | 034 |
| `variance` | `union` | 035 |
| constraint axis (`docs/constraints.md`, 8 forms) | `constraint` | 036 |
| size axis (`docs/schema/size-axis.md`, 6 forms) + computed values | `size_axis` | 037 |

That is 9 features × 2 directions = 18 GoogleTest sources (minus any that
would be empty, §5.2) and 18 ut sources → 18 + 36 = 54 targets, against 23
today. The ut sources are the doubling: each is compiled twice. Measured cost
on a representative suite is ~4 s per TU with `include/s2s.hpp`, and the two
builds are independent, so `-j` absorbs it. Recorded because it is the price
of the two-entry scheme and someone will ask.

`docs/schema/computed-values.md`'s four aliases (`predicate`,
`eval_bool_from_fields`, `parse_if`, `eval_size_from_fields`) are not their own
row anywhere; they belong to 037 in the write/derive direction and to 034
(`parse_if`) in the optional direction. Named here so 041 does not discover
them as orphans.

---

## 7. Migration order, and keeping a half-emptied tree valid

The hazard the brief names — `test/runtime/` and `test/constexpr/` must stay
buildable while nine slices strip them — is structural, and the fix is in 028.

1. **028 hoists** the GoogleTest `FetchContent` block and
   `add_struct_cast_test` from `test/runtime/CMakeLists.txt` up to
   `test/CMakeLists.txt`, adds the ut block and `add_ut_test` beside them, and
   creates `test/fields/CMakeLists.txt` and `test/internals/CMakeLists.txt`.
   CMake functions defined in a parent directory are visible in every
   subdirectory added after the definition, so `runtime/` and `fields/` can
   both call `add_struct_cast_test` for as long as both exist. Without this,
   slice 029 cannot register a GoogleTest target outside `runtime/`, and 041
   cannot delete `runtime/` without breaking every other directory.
   `add_constexpr_test` stays in `test/constexpr/CMakeLists.txt` and is
   deliberately not hoisted — nothing new will ever call it; it dies with the
   directory in 041.
2. **Every list is explicit, none is globbed.** A slice deletes its
   `add_struct_cast_test(foo)` / `add_constexpr_test(bar)` lines from the old
   directory in the same commit that deletes `foo.cpp`, and adds the
   corresponding lines to `test/fields/CMakeLists.txt`. A `CMakeLists.txt`
   whose every call has been removed is still valid CMake, and
   `add_subdirectory` on a directory holding only such a file is fine — so the
   old directories degrade to empty shells and stay green the whole way.
3. **041 deletes** the two directories and their `add_subdirectory` lines.
   Precondition: step 1 already moved everything shared out of them.

Slice-internal order, which keeps the "identical pass/fail" gate checkable:
record `ctest` output for the feature's existing entries → move the GoogleTest
cases → port the `static_assert`s → confirm the same set passes → *then* add
the new read-side compile-time cases as a separate commit. New coverage added
in the same commit as a move makes the gate unverifiable.

028's pilot: `test/internals/ut_modes_ct.cpp`, and it is permanent, not
scaffolding to delete later. It exists because a typo in
`target_compile_definitions` would leave both entries silently passing in ut's
default mode — the compile-time binary has no runtime output to notice its
absence by. The suite pins the modes apart:

```cpp
constexpr auto probe() { if consteval { return 42; } else { return 87; } }

"the compile-time entry really evaluates at compile time"_test = [] {
#ifdef UT_RUN_TIME_ONLY
  expect(eq(probe(), 87));
#else
  expect(eq(probe(), 42));
#endif
};
```

It deliberately does not include s2s: a harness proof that fails for library
reasons proves nothing. The s2s-side diagnostic 028 must record is already
captured in §1.

---

## 8. Slice template (029–037)

So no slice re-decides:

1. `test/fields/<f>_read.cpp`, `<f>_write.cpp` — GoogleTest, `TEST(<F>Read, …)`,
   include `s2s_test_utils.hpp`. Omit a file that would hold no cases.
2. `test/fields/<f>_read_ct.cpp`, `<f>_write_ct.cpp` — ut, `int main()`,
   include `<iostream>`, `<ut>`, `include/s2s.hpp`, `constexpr_memstream.hpp`;
   `using ut::expect; using ut::eq; using ut::operator""_test;`.
3. `test/fields/CMakeLists.txt` gains `add_struct_cast_test(<f>_read)` … and
   `add_ut_test(<f>_read_ct)` … .
4. The old sources and their registration lines are deleted in the same commit.
5. `ctest` green, and the feature's pass/fail set is compared to the recorded
   pre-move set before new cases are added.

---

## 9. Decisions on the standing categories

**State and data lifecycle.** Two places where partial completion is
observable. (a) *The migration itself*: the repository is the mutable state,
and the unit that must succeed or fail as a whole is one feature slice — a
commit that moves half a feature leaves a directory claiming coverage it does
not have. Reversibility is by `git revert` of a whole slice, which is only
meaningful if the slice is atomic; hence the rule that CMake registration,
source move and source deletion land in one commit. (b) *Test fixtures on
disk*: the GoogleTest cases write `test_input.bin` and read it back within one
case, so the state's lifetime is one test — except that all of them use the
same name in the same directory, which is the `-j` hazard §2.4 fixes. Nothing
here needs to be replayable or auditable; nothing persists past a `ctest` run
except the build tree.

**Error propagation.** The seams are: assertion → binary exit status → CTest
entry. The representation changes at each, and each mode changes it
differently — this is the whole reason for two entries rather than one. A ut
compile-time failure never reaches a process at all; it is a compiler
diagnostic, surfaced by the build the CTest entry drives, and its text is the
`LastTest.log`. A ut run-time failure is a printed line plus `SIGABRT` from a
static destructor, which CTest renders as `Subprocess aborted` rather than a
clean exit 1 — acceptable, and noted so nobody debugs the abort as if it were
a crash. A GoogleTest failure is exit 1 with the usual report. What each layer
knows about the one below: the CTest entry knows only pass/fail plus captured
output, which is exactly why the entry names (`…_ct_compile_time` vs
`…_ct_run_time` vs plain) have to carry the mode — the name is the only
diagnosis available before opening the log.

**Concurrency and ownership.** The test *code* is single-threaded with no
shared mutable state; every ut body is capture-free by rule (§5.3), and
capture-free is also what keeps compile-time evaluation on. The concurrency
that exists is CTest's own: `ctest -j` runs entries in parallel, and two
resources are shared across them — the build tree (owned by whichever
build-driving entry holds `RESOURCE_LOCK build_tree`) and the working
directory's `test_input.bin` (given a per-test working directory in §2.4).
Both are stated rather than assumed because both are invisible until a
parallel run corrupts something and looks like a flaky test.

**Reuse.** Reused rather than reinvented: the helper-function-per-test-kind
pattern (`add_struct_cast_test`, `add_constexpr_test`) that `add_ut_test`
extends; `compile_fail/`'s build-driving `add_test` idiom, which turns out to
be exactly what the compile-time entry needs; `FetchContent`, already the
project's dependency mechanism; `test/utils/constexpr_memstream.hpp`, which is
the pure-tier stream and needs no ut-specific replacement;
`constexpr_write.cpp`'s helper-function-plus-assertion-block shape, which is
what makes the port mechanical. Worth factoring for next time: `add_ut_test`
itself, and the hoist of shared declarations into `test/CMakeLists.txt` — the
next test kind after ut has an obvious place to go.

**Extension points.** Exactly one, deliberately: `add_ut_test(<stem>)`. Adding
a feature is two sources and two lines; adding a whole new pure-test directory
is a `CMakeLists.txt` with calls into helpers it does not own. The contract
that keeps it stable is the naming convention in §3.2 — `_ct_compile_time` and
`_ct_run_time` are load-bearing, because `ctest -R` selection depends on them.
Explicitly *not* extension points: no custom `ut::cfg<ut::override>` reporter
(§ build vs. buy), no shared "ut prelude" header, no per-feature CMake
variables. A later caller wanting different ut behaviour edits `add_ut_test`.

**Build vs. buy.** ut itself is settled upstream. Within this design: the
compile-time CTest entry was *not* built as a bespoke script — CMake's own
`--build --target` plus `RESOURCE_LOCK` covers it, and the repo already uses
that idiom. A custom ut configuration *was* evaluated and rejected: it would
remove the requirement that compared values be `std::clog`-streamable and
would let the run-time binary exit 1 instead of aborting, but ut's own example
of a custom config prints nothing at all on failure, so buying those two fixes
costs the failure messages that are the reason for adopting ut. The default
reporter is fine here because `s2s::error_reason` is unscoped and `failed_at`
is a `string_view` — checked, not assumed. `-DNTEST` was evaluated and
rejected on measurement (0.2 s across a whole TU). Rewriting `memstream` was
not considered: it already works in both directions.

**Abstractions introduced.** Four, each with the problem that forces it.
`add_ut_test` — a ut source must become two targets with two macros and two
differently-shaped CTest entries; written out per source it is nine lines × 18
sources, and the framework-separation invariant would live in prose instead of
code. `test/internals/` — `test/fields/` has to mean one entry per schema
construct for 041's audit to be mechanical; traits and containers are not
schema constructs. `test/fields/` itself — the spec's, not a new one. The
`ut_modes_ct.cpp` mode guard — the compile-time build produces no runtime
output, so a mis-wired macro is otherwise undetectable. Considered and
collapsed back into concrete code: a shared ut prelude header (four `#include`
lines and three `using`s per source do not need a header, and hiding them
makes the "no `using namespace ut`" rule invisible); a split of
`s2s_test_utils.hpp` (§4); a wrapper around `expect` for s2s
`std::expected` results (`expect(eq(res.has_value(), true))` is already one
line and reads better than a bespoke matcher).

**Alternatives rejected.**
*One binary per feature holding both frameworks* — settled upstream, and the
two mechanisms behind it were re-verified here (§1): the abort-from-destructor
and the TU-wide `static_assert`.
*One ut target per source, built in ut's default mode (both modes at once)* —
half the compile cost and one CTest entry, but a failure cannot be attributed
to a mode, and it forfeits the `ctest -R _ct_run_time` selector that makes
"show me the actual values" a single command.
*Compile-time targets `EXCLUDE_FROM_ALL`, built only by CTest* — closer to
`compile_fail/`, keeps `cmake --build` green, but serialises 18 compilations
behind `RESOURCE_LOCK` during `ctest` instead of letting `-j` build them, and
hides compile-time breakage from the ordinary build.
*Running the compile-time binary instead of driving its build* — rejected on
the reproduced false green (§2.2).
*ut sources compiling against `single_header/s2s.hpp`* — uniform with the
GoogleTest side, but points every compile-time diagnostic at generated code
(§2.3).
*`mutable` ut tests for the 59 `stringstream` write cases* — makes the spec's
literal routing rule work at the cost of a run-time-only ut binary that
duplicates GoogleTest and reports worse (§5).
*Renaming to `test/negative_compilation/` and `test/shipped_artifact/`* —
§3.1.
*A single `<feature>.cpp` per feature holding read and write* — the spec keeps
directions separate; the write suites are already the larger half and the
diff of a combined move would be unreviewable.

---

## 10. Where the repo or the issues contradict the spec

Flagged rather than silently absorbed.

1. **The routing rule.** "Touches a file" mis-routes the entire 59-case write
   suite, which uses `std::stringstream` and no file at all. Corrected in §5
   to "constant-evaluable". Affects the wording of 029–037's second acceptance
   criterion, not their intent.
2. **Issue 036 lists four sources unconditionally**
   (`constraint_{read,write}.cpp` and both `_ct`), but `field_validators.cpp`
   — its entire GoogleTest inheritance — is pure and moves wholesale to ut.
   One or both GoogleTest files may legitimately not exist. §5.2.
3. **Issue 028 says "no test is reorganised here" but also requires the
   GoogleTest/ut separation to hold.** It cannot deliver the second without
   moving the GoogleTest `FetchContent` block and `add_struct_cast_test` out
   of `test/runtime/`, which is a build reorganisation. §7 puts that hoist in
   028 deliberately; without it, 029 has nowhere to register a GoogleTest
   target and 041 cannot delete `test/runtime/`.
4. **Suite renaming goes slightly further than the spec authorises.** The spec
   scopes renaming to "names the move makes nonsensical" — that is the 27
   `S2STest` cases plus `MetaStructTest`, `MetaStructReadTest`,
   `FieldConstraintTest`, `RuntimeComputationTest`. The write side's
   `WriteTrivialFields`-style names are not nonsensical, but leaving them
   produces pairs like `TrivialRead` / `WriteTrivialFields` sitting in one
   directory, and re-raises the question in each of nine slices. This design
   applies the derivable rule (suite = PascalCase of the stem) to both
   directions. It changes no CTest entry name — entries are per binary here,
   so gtest suite names are not externally visible — and no coverage. If that
   is unacceptable, the fallback is: rename only the catch-alls and accept the
   mixed pairs.
5. **Issue 039's sweep misses `scripts/check_doc_examples.py`**, which names
   `test/single_header/` three times (in its module docstring and in the error
   message it prints to whoever adds an unbound example). 15 references across
   `docs/`, `README.md`, `AGENTS.md` and that script; 11 of them are `<!-- docs: -->`
   bindings.
6. **Issue 040's example cannot fail.** `test/hello/hello_s2s.cpp` uses
   `transform`/`transform_error` and then `return 0` unconditionally, so a
   parse failure prints a message and exits successfully — a CTest entry on it
   would pass while the example is broken, which is the exact failure mode 040
   exists to prevent. It also reads `sample.bin` from the working directory,
   which nothing generates; it needs a fixture the way `readme_read_example`
   has `make_doc_fixtures.py` with `FIXTURES_SETUP`. Both are prerequisites of
   "the example compiles and runs clean", not extras.
7. **`AGENTS.md` will be stale in three places** once this lands: line 20
   ("split into `test/runtime/` and `test/constexpr/`"), line 36
   (`test/single_header/`), and the Overrides bullet requiring coverage "by
   both `test/runtime/` and `test/constexpr/`". 039 and 040 each touch
   `AGENTS.md`; the `test/runtime`/`test/constexpr` sentences belong to 041,
   which is the slice that makes them false.
8. **The untracked root `qlib_ut.hpp` is already gone** — 028's last
   acceptance criterion is satisfied before 028 starts. The `git status`
   snapshot that showed it is stale.

---

## 11. Residual risks

- **`test/constexpr/static_tests_1.cpp` (69 assertions) is against an API
  several generations old.** Issue 038 calls it a rewrite, and it is: expect
  the count to drop, and expect the honest answer for
  `field_value_constraints_traits.hpp` to be deletion, since no header
  includes it and the amalgamation therefore never sees it. That is 038's call
  to record, not this design's to pre-empt.
- **`field_list_metadata.cpp` carries 47 assertions** and is the file gcc 13
  cannot compile (per the root `CMakeLists.txt` comment). Porting it to ut
  keeps that constraint — the ut compile-time build is the same constant
  evaluation. No change, but do not read a new failure there as a porting bug.
- **Compile-time budget.** 36 ut targets at ~4 s each plus 18 GoogleTest
  binaries. If a full rebuild becomes painful, the lever is dropping the
  `_run_time` targets for suites that never fail informatively — not merging
  the two modes into one target.
- **No CI runs `ctest`.** `.github/workflows/` has only `docs.yml`. Every gate
  in this work — the pass/fail parity gate especially — is enforced by a human
  running `ctest` locally. Worth its own issue; out of scope here.
