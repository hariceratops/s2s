# [feat] Union alternatives carry a constraint

`to_field_choice` hardcoded `no_constraint<T>{}` for every alternative, so a
`match_case` whose payload should be validated could not say so. 048 moved that
default onto the tag; this slice makes it a pack option the schema author
supplies:

```cpp
s2s::match_case<0x01, s2s::as_trivial<u32, 4_B, s2s::range{1u, 99u}>>
```

The constraint is checked against the tag's `::type` — `std::vector<T>` for an
`as_vec` alternative, not `T`.

Enforcement needs two explicit call sites, and the reason is worth stating
because it is easy to assume otherwise: the struct-level fold
(`struct_cast_impl.hpp:37`, `stream_cast_impl.hpp`) runs
`fields::constraint_checker` over the fields of a `struct_field_list`, and for
a union the field it sees is the outer `union_field`. The inner alternative
field never passes through it. `maybe_field` has exactly this problem — its
constraint is rewritten to `no_constraint` over the optional — and solves it
with an explicit check at `field_writer.hpp:195`, with a comment explaining
why. This follows that precedent on both paths.

A constraint that is accepted but never runs is the failure mode this slice
has to rule out, on the read path and the write path independently.

Depends on 049.

Spec: `dev/specs/union-alternatives-have-no-option-pack.md`.

## Acceptance Criteria
- Every type tag admits a constraint entry in its pack, resolved through
  `constraint_of_pack`, checked against the tag's `::type`.
- The four tags with no size entry — `as_fixed_arr`, `as_fixed_string`,
  `as_struct`, `as_arr_of_records` — gain their trailing pack **in this
  slice**, having kept their current arity through 049. Moved here by the
  design (§6.1): a pack that admits nothing is not a pack, so their arity
  changes where the constraint that justifies it is enforced. Their migration
  is empty for the same reason 049's is.
- A read whose alternative payload fails its constraint is rejected with a
  `cast_error` carrying `error_reason::validation_failure` and the union
  field's id in `failed_at`. `cast_error` gains no member; which alternative
  failed is not reported.
- A write whose alternative payload fails its constraint is rejected the same
  way, before any bytes reach the stream.
- Both are covered in both test tiers, per
  `dev/specs/compile-time-test-tier.md`.
- **A test proves the check is not vacuous on each path independently.**
  Removing the read-side check must fail a read-side test, and removing the
  write-side check must fail a write-side test. One test covering both paths
  would pass with either call site missing.
- An alternative that declares no constraint keeps `no_constraint<type>{}` and
  is not charged for a check it did not ask for.
- Two constraint entries in one pack is a compile error, via the existing
  duplicate-count assertion, with a registered `must_not_compile` case.
- The constraint composes with a size entry in either order.
- The documentation's union reference shows a constrained alternative and
  states that the constraint applies to the tag's resulting type, not its
  element type.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.
