# [chore] document the discovered-length size form

The size axis is documented as a closed set of ways to state a length before the
read. This adds one that states it afterwards, which is a change to what the axis
*is*, not another row in a table — so the prose that frames the axis needs
revisiting, not just extending.

Follows 061's shape for the byte-order axis.

Three things are easy to leave out, and each is the kind of thing a reader only
discovers by being bitten:

- **The write-side rule from 066.** A value containing the delimiter fails to
  write. No other size form constrains what a value may contain, so a reader has
  no reason to expect it.
- **The bound's changed role.** For every other field `max_bytes` is a guard
  against a corrupt declared length; here it is the termination condition, and a
  field with no delimiter in it reads to the ceiling before failing. It counts the
  value, not the wire.
- **The cost.** This field kind reads per byte and the others do not. The spec's
  Technical Approach carries the reasoning, including why `peek` is the wrong
  primitive and `read_until` the right one; that belongs somewhere durable, and
  the frozen spec is not where a reader looks.

Spec: `dev/specs/the-size-axis-cannot-say-read-until-a-byte.md`.

## Acceptance Criteria
- The descriptor table in `docs/schema/index.md` gains the form, and the size-axis
  page's framing prose no longer claims every length is known before the read.
- `docs/` states the write-side rejection from 066, the bound's role as the
  termination condition, and its value-not-wire accounting.
- The per-byte read cost is stated plainly rather than omitted, with the
  `read_until` note recorded so the reasoning survives the spec being archived.
- The three failure reports are documented together: delimiter found, stream dry
  (`buffer_exhaustion`), bound reached (`delimiter_not_found`), plus the write-side
  reason from 066.
- An ELF- or Git-shaped example — a bare undeclared NUL-terminated string with
  nothing but `max_bytes` bounding it — appears under `test/doc_examples/` with its
  `<!-- docs: ... -->` binding and a `// docs-begin` / `// docs-end` region.
- The example uses a realistic wire format and real field names, and reads through
  `std::ifstream` opened binary rather than a `std::stringstream`, per AGENTS.md.
- Every new page appears in `docs/mkdocs.yml`'s nav, so `docs_nav_lists_every_page`
  passes.
- `README.md`'s roadmap ticks "Read-Until Delimiter[s]" only if the sentinel
  sequence feature does not also claim that line; if it does, the line is split so
  each feature ticks what it delivered.
- `ctest` is green tree-wide, including `doc_examples_match`.

## Review 2026-10-10
- `docs/schema/size-axis.md:112-123` ("Three read outcomes, one write outcome") restates the error rationale that `docs/errors.md` owns; trim it to a link, per "nothing is documented twice".
