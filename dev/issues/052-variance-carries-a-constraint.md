# [feat] variance carries a constraint

`union_field` builds its own base field with
`no_constraint<typename type_deducer::variant>{}` hardcoded at
`include/field/field.hpp:103`, so a `variance` field as a whole cannot be
constrained — independently of whether its alternatives can be. A schema can
say what each alternative must look like (050) but not what the resolved
variant must be.

```cpp
s2s::variance<"body", guide, some_constraint_on_the_variant>
```

This is the gap the source brief never named; it turned up while verifying the
per-alternative one. It is listed last because it reads that way, not because
it depends on anything: it touches `variance` in `field_descriptors.hpp` and
`union_field` in `field.hpp`, and nothing in 048–051. **It can be pulled
forward** ahead of the migration if a quick slice is wanted.

Unlike an alternative's constraint, this one needs no new call site. The
struct-level fold at `struct_cast_impl.hpp:37` runs
`fields::constraint_checker` over every field of the list, and a
`union_field` *is* one of those fields — verified, not assumed. The
alternative's constraint needs an explicit check precisely because the inner
field is the one the fold cannot see.

Spec: `dev/specs/union-alternatives-have-no-option-pack.md`.

## Acceptance Criteria
- `variance` takes a constraint-only trailing pack, resolved through
  `constraint_of_pack` against the deducer's `variant` type, replacing the
  hardcoded `no_constraint<variant>{}`.
- A read whose resolved variant fails the constraint is rejected with a
  `cast_error` carrying `error_reason::validation_failure` and the union
  field's id, at the ordinary fold point — no new call site in
  `read_variant_impl`.
- The same on the write path.
- Both covered in both test tiers, and a test proves the check is not vacuous.
- A size or bound entry in `variance`'s pack is a compile error: a union's own
  size is `size_dont_care` and it drives no allocation of its own.
- A `variance` declaring no constraint behaves exactly as it does today.
- Two constraint entries is a compile error, via the existing duplicate-count
  assertion.
- The union-level constraint and a per-alternative constraint (050) both apply
  when both are declared, and a test covers them together — the interaction is
  the part most likely to be got wrong.
- The documentation's union reference distinguishes the two: one constrains the
  resolved variant, the other constrains a single alternative's payload.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.
