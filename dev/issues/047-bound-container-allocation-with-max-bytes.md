# [feat] Bound container allocation with a schema-declared max_bytes

A variable-sized field's length comes off the wire and is handed straight to
`resize`, so a corrupt `u32` allocates gigabytes before a byte of payload is
read. This adds `max_bytes<N>`, a per-field ceiling the schema declares, checked
against the allocation before it happens and rejected as a recoverable
`cast_error`. Safety is on by default: a field that declares nothing still gets
a 16 MiB ceiling, and `max_bytes` raises or lowers it per field.

The option is declared on the **container**, never on the length field. A single
length can size several containers of different element widths — `fanout` in
`test/schema/size_axis_read_ct.cpp` has one `"len"` sizing both a `vec<u16>` and
a `vec<u32>` — and a `size_from_fields` length has no length field to annotate
at all. The thing being bounded is the allocation, which belongs to the
container, and that is also what lets the bound be denominated in bytes rather
than elements.

It is a trailing option in the pack `field_descriptors.hpp` already classifies,
not a new descriptor, so it composes with a size and a constraint in any order:

```cpp
s2s::vec_field<"payload", u8, s2s::len_from_field<"n">, s2s::max_bytes<4096>>
```

Depends on 046 for the checked product.

Spec: `dev/specs/unbounded-resize-from-wire-length.md`.

## Acceptance Criteria
- A length whose byte count exceeds a declared `max_bytes<N>` is rejected with a
  `cast_error` before any allocation proportional to that length occurs, at both
  sites that resize today: `read_native`'s `variable_sized_buffer_like` overload
  in `read_impl.hpp` and the `vector_of_records` path in `field_reader.hpp:153`.
- The bound is `count * sizeof(element)` uniformly for `vec_field`, `str_field`
  and `vector_of_records` — memory, not wire size, so records carrying
  variable-length subfields need no special case.
- `max_bytes<N>` is inclusive: a byte count equal to `N` is accepted, `N + 1` is
  rejected. Below-bound, at-bound and over-bound are each covered for all three
  container kinds, in both test tiers.
- A new `error_reason` enumerator names this rejection distinctly from
  `buffer_exhaustion`, appended so existing enumerator values are unchanged.
  `cast_error` gains no member — the reason code is the entire enrichment.
- `max_bytes` on a fixed-size field is a compile error, expressed through the
  pack classifier's concept mechanism rather than a bolted-on `static_assert`,
  with a `test/must_not_compile/` case. Silent inertness is not acceptable: it
  would let a schema author believe they had bounded something they had not.
- Two `max_bytes` entries in one pack is a compile error, via the same
  duplicate-count assertion `pack_options` already applies to sizes and
  constraints.
- The option composes with a size and a constraint in any order.
- **Omitting it does not mean unbounded.** A field that declares no `max_bytes`
  gets `default_max_bytes`, 16 MiB, so a schema written without thinking about
  this is still protected. Safety is on by default.
- `S2S_DEFAULT_MAX_BYTES` is the single global knob: raising it raises every
  default, and setting it to `SIZE_MAX` turns the defaults off wholesale. It
  cannot reach a declared `max_bytes` — a default is the library's guess, a
  declaration is the author's intent, and no build flag discards the latter.
  There is no second on/off switch or profile.
- The bound applies only where wire input drives an allocation — `vec_field`,
  `str_field`, `vector_of_records` — and this is a promise, not an omission.
  Fixed-size fields, `fixed_array_field`, `array_of_records` and `c_arr_field`
  have extents fixed by template parameters that no stream can influence.
- The default, how to raise it, and the fact that a declared bound survives the
  global knob are discoverable in the documentation, not only implicit in
  `max_bytes`'s reference entry.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.
