# [chore] Rename compile_fail/ and single_header/ to say what they verify

Both directories are named after their mechanism rather than what they check:
`compile_fail/` verifies negative compilation, `single_header/` verifies that
the amalgamated header works and that every documented example compiles and
matches its fenced block. Exact replacement names are design's call.

The `single_header/` rename is a docs-wide sweep, not a `git mv`. That path is
hard-coded in every `<!-- docs: test/single_header/… -->` binding across
`docs/`, in `AGENTS.md` line 36, in the registrations inside
`test/single_header/CMakeLists.txt`, and — easily missed — in
`scripts/check_doc_examples.py`, which hard-codes it in three places including
the error message it prints when a binding is wrong. `ctest` enforces the
binding, so a missed reference fails the build rather than rotting silently,
but the checker's own hard-coded path is the one place that would keep passing
while pointing at a directory that no longer exists.

Design chose the replacement names: `test/compile_fail/` → `test/rejected_misuse/`,
`test/single_header/` → `test/as_shipped/`. See `dev/design/compile-time-test-tier.md`.

Depends on: 028. Independent of 029-038.

## Acceptance Criteria
- Both directories are renamed; the new names state what is verified, not how.
- Every `<!-- docs: … -->` binding in `docs/` points at the new path.
- `AGENTS.md`'s description of the doc-example convention points at the new
  path.
- `test/CMakeLists.txt` and the renamed directories' own CMakeLists refer to
  the new paths.
- `scripts/check_doc_examples.py` refers to the new path everywhere, including
  in the guidance text of its failure message.
- `doc_examples_match` and `docs_nav_lists_every_page` pass.
- A deliberately broken binding is confirmed to still be caught after the
  rename, and the error message names the new directory — the checker passing
  is not by itself proof it is still checking anything.
- `grep -rn 'test/single_header\|compile_fail' .` returns nothing outside
  `dev/`, `.git/` and `build/`.
- The set of tests registered is unchanged — this renames, it does not add or
  remove a case. `compile_fail/` still holds exactly its one existing case;
  expanding it is explicitly out of scope.
- `ctest` green.

## Notes 2026-08-12

Done. `ctest` 104/104, the same 104 entries as before the rename — this moved
things, it did not add or remove a case.

`test/compile_fail/` → `test/rejected_misuse/`, `test/single_header/` →
`test/as_shipped/`. Swept: 12 `<!-- docs: … -->` bindings across `docs/` and
`README.md`, `AGENTS.md`, `test/CMakeLists.txt`'s two directory variables,
both renamed directories' own CMakeLists, `scripts/check_doc_examples.py`, and
one comment in `test/fields/size_axis_read_ct.cpp` pointing at the negative
half of a case it covers positively.

Test and target names carried the old mechanism names too, so they moved with
the directories: `add_compile_fail_case`/`add_compile_pass_case` are now
`add_rejected_case`/`add_accepted_case`, and the four cases read
`rejects_assigning_a_derived_len`, `rejects_assigning_a_derived_discriminant`,
`rejects_duplicate_match_values`, `accepts_reading_a_derived_field`.
`single_header_roundtrip` is `as_shipped_roundtrip`. The root `single_header/`
directory and `SINGLE_HEADER_DIR` are untouched — that is the amalgam output a
consumer includes, not a test directory.

### The checker was proven to still check, twice

`doc_examples_match` passing after a rename proves nothing on its own: the
script's own hard-coded path is the one reference that would keep passing
while pointing at a directory that no longer exists. Both failure paths were
triggered deliberately and both name the new directory:

```
docs/constraints.md:73: bound to 'test/as_shipped/no_such_example.cpp', which does not exist.
docs/constraints.md:72: complete program has no '<!-- docs: <path> -->' binding. Back it
with a source file under test/as_shipped/, register that file in
test/as_shipped/CMakeLists.txt, and bind the block to it.
```

The second is the guidance text, which is where two of the script's three
hard-coded paths lived and where a stale path would have been invisible.

### Also corrected here

`AGENTS.md` still described the tree as "mid-reorganisation from execution-mode
directories (`test/runtime/`, `test/constexpr/`)". Both were retired in 037 and
038, so that paragraph now describes the four directories that exist.

`.claude/settings.local.json` still contains the old paths in 17 places. Those
are historical Bash-permission records from earlier sessions, several pointing
at scratchpad directories that no longer exist. Rewriting them would be editing
a log of what was approved, not a reference to fix. Left alone deliberately;
this is the one place the acceptance criterion's grep still matches.
