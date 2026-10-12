# [chore] document views, including what they do not do

A view is the first part of the library whose API carries caveats the type
system does not check. If the docs bury them in one corner, a reader meets them
by debugging a dangling view, and "zero-copy" — the phrase everyone will reach
for — is simply not what this does. This slice writes the guide page and the
reference entries, with the caveats stated at each point of use rather than
once.

The saving is the fields not touched and nothing else: each field read costs
what the parser costs for that field, and a byte-swapped field must be returned
by value. For a caller reading every field of one record it buys close to
nothing, and the docs say so.

Spec: `dev/specs/reading-two-fields-costs-a-whole-record.md`.

## Acceptance Criteria
- A guide page and reference entries under `docs/` for the entry points, the
  accessors, the sub-view, the record-width constant and the fixed-layout
  concept, listed in `docs/mkdocs.yml`; `docs_nav_lists_every_page` passes.
- The docs state plainly that a view is not zero-copy: it saves the fields
  that are not touched and buys little for a caller who reads every field.
- **The unsafe contract** appears in each entry point's and accessor's
  reference entry and on the guide page, not once in a caveat section:
  - lifetime is wholly the caller's — nothing in the library or the language
    checks that the bytes outlive the view, and using a view after its bytes
    are gone is undefined behaviour;
  - bounds are checked once at creation — at compile time for a static extent,
    against the span's length for a dynamic one — and a span built from a wrong
    pointer or size cannot be detected;
  - `array_of_records` indexing is unchecked, `[i]` with `i >= N` is a
    precondition violation that can silently return garbage because trailing
    bytes are permitted.
- The unsafe contract also says that bytes changed after creation are read
  unvalidated: constraints are checked once, at creation (design F5).
- The README roadmap line "Zero copy views" is reworded to match the
  not-zero-copy statement (design F9), and `AGENTS.md`'s description of
  `test/schema/` is widened to cover the view's tests (design F6).
- The `c_arr_field` / `c_str_field` read type (`std::array`) and the
  accepted/rejected schema kinds are documented, with the rejected kinds and
  the reason for each.
- The 074 sanitizer command is documented where a developer will look.
- Every program shown follows the project's doc-example binding rule
  (`test/doc_examples/`, `doc_examples_match`).
- `ctest` is green tree-wide, including `doc_examples_match`; the README and
  `AGENTS.md` are checked for statements this feature makes stale, and
  `single_header/s2s.hpp` is regenerated if any header changed.
