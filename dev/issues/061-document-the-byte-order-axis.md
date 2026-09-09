# [chore] document the byte-order axis

The feature adds a schema axis and a public entry point, and the documentation
currently states that byte order is chosen by which function the caller calls.
That statement is true of every slice until the last one lands, which is exactly
what a per-slice documentation pass does not catch.

Both deferrals are documented as limitations rather than left silent: a resolved
order always governs the rest of the file (subtree-scoped resolution is not
available), and a schema may declare at most one announcing record.

Spec: `dev/specs/byte-order-is-data-not-a-template-parameter.md`.

## Acceptance Criteria
- `docs/reading.md` covers the announcing record and `struct_cast`, including
  that resolution happens at a point in the byte stream and applies to
  everything read after it.
- `docs/writing.md` covers the marker deciding the output order, and that it is
  ordinary settable data rather than a frozen field.
- `docs/schema/` documents the declaration surface and all three deduction
  forms.
- `docs/reference.md`'s declaration block covers the new spelling and entry
  point.
- The restriction that an announcing record's own fields must be order-agnostic
  is documented, with its narrow boundary stated — it does not extend to the
  containing record's later siblings.
- The boundary is documented in its **true** form, not its intended one: a field
  read before the announcement completes is read at an unspecified order, and
  the library checks only the announcing record's own fields. A field placed as
  an *earlier* sibling of the announcing record is therefore not diagnosed. This
  is a known gap, deliberately shipped (design §4.3, finding 3); say so rather
  than implying the check is total.
- Both deferrals are documented as limitations, noting that lifting either is
  additive.
- Any statement that byte order is fixed by the choice of entry point is
  corrected wherever it appears, including `README.md`.
- `scripts/check_doc_examples.py` and `scripts/check_nav.py` pass; worked
  examples compile under the doc-examples test.
- `ctest` is green tree-wide.
