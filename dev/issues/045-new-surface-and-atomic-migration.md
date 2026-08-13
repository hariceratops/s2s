# [feat] New schema surface, and the atomic migration onto it

With sizes value-shaped (044), the terse surface becomes expressible: a
`basic_field`'s size defaults to `sizeof(T)`, the trailing parameters become an
order-independent option pack classified by kind rather than fixed position,
and the `field_size<>` / `with_fields<>` wrappers stop being something the user
types. `basic_field<"version", u16, field_size<fixed<2>>>` becomes
`basic_field<"version", u16>`, and where a width genuinely differs from the
type, `basic_field<"version", u16, 2_B>`.

Wrapper elision reaches depth-first, not stopping at `variance`'s boundary:
`match_field`, `match_case`, `type_switch`, `branch`, `predicate` and the
`as_*` alternative tags all carry `field_size` or `with_fields` themselves.
Inside `variance` these changes are spelling-only and behaviour-preserving —
they must neither suggest nor foreclose an answer to how a caller reads a
variance field's held alternative, which is a separate unsettled question.

The spec forbids the old and new spellings coexisting in the tree, and there
are no external users to keep the old forms alive for, so removal and migration
land in the same commit as the surface: all 72 files spelling `field_size<`
(12 in `include/`) and the 23 spelling `with_fields<`, plus `amalgam.py`
regenerating `single_header/s2s.hpp` so `test/shipped_header/` never sits on a
stale artifact. This is one commit by requirement, not by preference — it is
the reason this slice cannot be thinned.

Depends on 044.

Spec: `dev/specs/schema-api-verbosity.md`.

## Acceptance Criteria
- `basic_field`'s size defaults to `sizeof(T)` when omitted, and a constraint
  can be given without spelling the size.
- The trailing option pack is order-independent and classified by named
  concepts; an entry that is neither a size nor a constraint is rejected by a
  concept that states what was expected, with `static_assert` used only where a
  concept cannot express the shape.
- A `test/must_not_compile/` case demonstrates that diagnostic.
- Both the wrapped and naked spellings are accepted at every user-facing alias
  during the change, and the wrapped forms are gone by the end of it.
- Elision reaches inside `variance`; its read and write API is unchanged, and
  no new combinator sugar is introduced for `maybe` or `variance`.
- No occurrence of the old spelling remains anywhere under `include/`,
  `test/`, `docs/`, or `examples/`.
- `single_header/s2s.hpp` is regenerated in the same commit;
  `shipped_header` and `doc_examples_match` are green.
- `ctest` is green tree-wide, including `*_compile_time` and `*_coverage`.
