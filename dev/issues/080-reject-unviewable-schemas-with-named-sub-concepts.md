# [feat] a schema that cannot be viewed is rejected, naming the part that fails

075–079 grew the accepted subset one descriptor kind at a time. This slice
completes the rule from the other side: what is *not* a fixed layout, and how
the author is told. A view accepts a strict subset of the schema language — the
first entry point that does — so a schema that fails it must be a compile
error that says why, never a run-time surprise.

Rejected: `vec_field`, `str_field` and `vector_of_records` (their size axis is
always run-time or discovered, so every later offset stops being a constant);
every `maybe`; every `variance`, including one whose alternatives all share a
width (a deliberate exclusion, listed in the spec's Non-Goals); and any schema
containing `announces_byte_order` at any depth.

The rule is a **pure concept** built from **named sub-concepts** — for example
every field has a compile-time width, no optional or union field, no announcing
record — and it must evaluate to `false` for a rejected schema, never
hard-error. That is a deliberate divergence from the byte-order precedent
(`fixed_order_schema` / `self_announcing_schema` in
`include/field_list/announcing_record.hpp`), which is backed by a struct of
`static_assert`s: evaluating those on a bad schema is itself the error, so they
cannot be tested negatively. Diagnostic readability instead comes from the
failing sub-concept's name appearing in the compiler's unsatisfied-constraint
report. The design records this divergence and why.

Spec: `dev/specs/reading-two-fields-costs-a-whole-record.md`.

## Acceptance Criteria
- The named sub-concepts and the composing concept are exercised as traits in
  the `ut` suite: `static_assert(concept<accepted>)` and
  `static_assert(!concept<rejected>)`, for the concept and for each
  sub-concept.
- The concept is `false`, not a hard error, for each of `vec_field`,
  `str_field`, `vector_of_records`, `maybe`, `variance` (including a
  same-width one) and `announces_byte_order`.
- One `test/must_not_compile/` case per rejected kind, registered with
  `add_rejected_case_matching` on the failing sub-concept's name, so each fails
  for its intended reason and not for an unrelated error.
- One further case with the offending field nested a record deep (and one
  inside an `array_of_records` element), proving the check descends.
- The concept contains no `static_assert`.
- `ctest` is green tree-wide, including the new must-not-compile entries;
  `single_header/s2s.hpp` is regenerated in the same commit.
