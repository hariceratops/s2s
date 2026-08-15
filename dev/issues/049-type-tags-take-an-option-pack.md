# [feat] Type tags take an order-independent option pack

Type tags spell their size positionally — `as_vec<u8, len_from_field<"n">>`,
`as_trivial<u32, 4_B>` — while every ordinary descriptor took an
order-independent trailing pack when `dev/specs/schema-api-verbosity.md`
landed. This brings the tags onto that same shape, so a tag's size is a pack
entry rather than a fixed second argument:

```cpp
s2s::as_vec<u8, s2s::len_from_field<"n">>   // still valid
s2s::as_trivial<u32, 4_B>                   // still valid
```

Both keep compiling because a one-entry pack is what they already look like.
What changes is that the slot is no longer positional, which is what lets 050
and 051 add a constraint and a bound to it in any order. Order-independence is
a property of the whole option set, not of a suffix of it.

This slice admits **size entries only**. A constraint or bound entry is
rejected until the slice that enforces it lands — a tag that accepted an
option it silently ignored would be worse than one that rejects it, for the
same reason `dev/issues/047` gives for rejecting `max_bytes` on a fixed-size
field.

Every tag signature changes, so every union schema that spells one is migrated
in the same commit, atomically — the same call
`dev/specs/schema-api-verbosity.md` made for the equivalent break, and for the
same reason: the library has no external users of the old spelling.

Depends on 048.

Spec: `dev/specs/union-alternatives-have-no-option-pack.md`.

## Acceptance Criteria
- `as_trivial<T, opts...>`, `as_vec<T, opts...>`, `as_string<opts...>` and
  `as_vec_of_records<T, opts...>` take size as a pack entry, resolved through
  `field_descriptors.hpp`'s existing `size_of_pack` rather than a parallel
  mechanism.
- `as_fixed_arr<T, N, opts...>` and `as_fixed_string<N, opts...>` keep `N`
  positional — it is an element count, not a size value — and take a trailing
  pack, mirroring `fixed_array_field` / `fixed_string_field`.
- `as_struct<T, opts...>` and `as_arr_of_records<T, N, opts...>` take a
  trailing pack with no size entry, mirroring `struct_field` /
  `array_of_records`.
- `as_trivial`'s `requires fixed_size_like<size_type_of<S>> &&
  (deduce_field_size<S>{}() <= sizeof(T))` is re-expressed against the size
  resolved from the pack, the same relocation `basic_field` already made. The
  constraint must still reject an oversized size, with a test proving it — a
  relocation that quietly stops constraining is the failure mode here.
- A constraint or bound entry in a tag's pack is a compile error in this
  slice, through the per-element concept mechanism. 050 and 051 widen what is
  admitted, each alongside the enforcement that makes it meaningful.
- Two size entries in one pack is a compile error, via the same duplicate-count
  assertion `pack_options` already applies, with a registered
  `must_not_compile` case.
- Every union schema in `test/`, `examples/` and `docs/` is migrated in this
  commit. No compatibility shim for the positional spelling, and no
  deprecation period.
- The documentation's union and type-tag reference describes the pack
  spelling, including that entries are order-independent.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.
