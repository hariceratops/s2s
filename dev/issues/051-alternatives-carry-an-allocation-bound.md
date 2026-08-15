# [feat] Union alternatives carry an allocation bound

`dev/issues/047` bounded every variable-sized field's allocation against a
corrupt wire length, with `max_bytes<N>` overriding a 16 MiB default. Union
alternatives got the default but no way to state their own: a vector
alternative is protected, but a legitimately large one is rejected with no
recourse short of `S2S_DEFAULT_MAX_BYTES`, which moves every other default
with it.

This makes the bound a pack option on the tags where an allocation is actually
driven by wire input:

```cpp
s2s::match_case<0x02, s2s::as_vec<u8, s2s::len_from_field<"n">,
                                      s2s::max_bytes<4096>>>
```

Depends on 049. Independent of 050 — they widen the same pack along different
axes and can land in either order.

Spec: `dev/specs/union-alternatives-have-no-option-pack.md`.

## Acceptance Criteria
- `as_vec`, `as_string` and `as_vec_of_records` admit a bound entry, resolved
  through `bound_of_pack` and threaded to the same read-path check 047 put in
  place — an oversized alternative is rejected before any allocation
  proportional to the wire length occurs, not after.
- The rejection carries the same `error_reason` 047 introduced for an
  over-bound length. No new enumerator, no `cast_error` member.
- The bound is `count * sizeof(element)`, inclusive, matching 047 exactly: a
  byte count equal to `N` is accepted, `N + 1` rejected. Below-bound, at-bound
  and over-bound are each covered, in both test tiers.
- A bound entry on `as_struct`, `as_arr_of_records`, `as_trivial`,
  `as_fixed_arr` or `as_fixed_string` is a **compile error**, through the same
  per-element concept mechanism `bound_pack_option_like` already uses — not a
  runtime check and not a bolted-on `static_assert`. Registered
  `must_not_compile` cases cover it, analogous to
  `rejects_bound_on_fixed_size_field`.
- **The registered negative cases fail on their own diagnostic.** A case that
  fails for an unrelated reason — an undeclared type, a typo — passes the
  harness while testing nothing. Verify each against a no-`CASE` control build.
- An alternative declaring no bound still gets `default_max_bytes`. Omission
  does not mean unbounded, here as everywhere else.
- A declared bound on an alternative is not clamped by `S2S_DEFAULT_MAX_BYTES`
  in either direction — a default is the library's guess, a declaration is the
  author's intent.
- Two bound entries in one pack is a compile error, via the existing
  duplicate-count assertion.
- The bound composes with a size entry, and with a constraint if 050 has
  landed, in any order.
- The documentation's union reference covers bounding an alternative and points
  at the allocation-limits material 047 added.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.
