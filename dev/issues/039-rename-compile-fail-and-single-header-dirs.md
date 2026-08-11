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
