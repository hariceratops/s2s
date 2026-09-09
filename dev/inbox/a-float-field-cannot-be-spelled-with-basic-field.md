# A scalar float field cannot be spelled with `basic_field`

Found on 2026-09-06 while landing
`dev/issues/053-runtime-byte-order-in-the-read-path.md`, which needed `float`
fields to prove the new `byteswapped` path. Not acted on there — 053's contract
was the engine change, and this is a descriptor-surface gap that predates it.

## What was hit

`basic_field` is constrained to `integral<T>`, so

```cpp
s2s::basic_field<"a", float, 4_B>
```

does not compile. The tests added in 053 spell it

```cpp
s2s::field<"a", float, 4_B, s2s::no_constraint<float>{}>
```

instead — reaching past the descriptor alias to the underlying construct, and
restating a `no_constraint` that every other field kind gets for free.

## Why it is odd rather than merely missing

The engine has no problem with floats. `trivial<T>` admits them, `read_impl`
and `write_impl` handle them, and after 053 `byteswapped` swaps them through
the unsigned integer of the same width. A float field round-trips correctly in
both byte orders — there is just no pleasant way to declare one.

`as_trivial` reaches floats inside a union tag's option pack, so the surface is
inconsistent as well as incomplete: the same type is declarable in one context
and not in the sibling context.

## Open questions

- Does `basic_field` relax from `integral` to `trivial`, or does a separate
  alias appear beside it? Relaxing is smaller but changes what an existing
  spelling accepts.
- `long double` is rejected by `byteswapped` (no portable wire form, no
  matching unsigned width). Whatever the descriptor does, it should reject it
  at the schema rather than at the leaf, where the diagnostic is worse.
- Is there a reason `integral` was chosen deliberately that this brief has not
  found? The constraint predates the write path and nothing records why.
