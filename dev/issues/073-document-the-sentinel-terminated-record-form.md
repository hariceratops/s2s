# [chore] document the sentinel-terminated record sequence form

`vector_of_records` is documented as taking its element count from the size
axis. This adds a way to end a run without a count at all, which changes what
readers should expect from the construct — so the descriptor table and the
surrounding prose both need the new row, not just an addition at the bottom.

Follows 068's shape for the discovered-length size form. Three things are
easy to leave out, and each is the kind of thing a reader only discovers by
being bitten:

- **The write-side rule from 072.** An element matching the sentinel value
  fails to write. Nothing else in `vector_of_records` constrains what an
  element's fields may contain, so a reader has no reason to expect it.
- **The two compile-time rejections from 070.** Combining
  `until_field_equals` with a size-axis count, and a terminating element that
  cannot synthesise correctly, both fail to compile — a reader designing a
  schema needs to know these are structural rules, not runtime behaviour to
  discover by testing.
- **The bound's nesting consequence.** The allocation bound applies per run,
  not across a nest, so the worst-case footprint of a sentinel-terminated run
  nested inside another is the *product* of the two bounds, not the outer
  bound alone. Stated plainly rather than implied as a stronger guarantee
  than the engine actually gives.

Spec: `dev/specs/a-record-sequence-cannot-end-at-a-sentinel.md`.

## Acceptance Criteria
- The descriptor table in `docs/schema/index.md` gains the form, alongside
  the size axis's framing prose updated to state that a run may end at a
  sentinel element instead of a resolved count.
- `docs/` states the write-side rejection from 072, both compile-time
  rejections from 070, and the per-run bound's nesting consequence from 069.
- The three read-side outcomes (sentinel found, `buffer_exhaustion`,
  `sentinel_not_found`) are documented together, alongside the write-side
  `found_sentinel_in_sequence` reason.
- A GIF-shaped example — `image_data` with a `blocks` field terminated by a
  zero-length sub-block — appears under `test/doc_examples/` with its
  `<!-- docs: ... -->` binding and a `// docs-begin` / `// docs-end` region.
- The example uses a realistic wire format and real field names, and reads
  through `std::ifstream` opened binary rather than a `std::stringstream`,
  per `AGENTS.md`.
- Every new page appears in `docs/mkdocs.yml`'s nav, so
  `docs_nav_lists_every_page` passes.
- `README.md`'s roadmap ticks "Read Until a Sentinel Record".
- `ctest` is green tree-wide, including `doc_examples_match`.

## Review 2026-10-10
- `docs/schema/size-axis.md:238-250,292-299` repeats the `found_sentinel_in_sequence` and `sentinel_not_found` rationale that `docs/errors.md:45-56` owns; trim to a link, per "nothing is documented twice".
