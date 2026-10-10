# A record sequence can end at a count, never at a sentinel

PRD. Supersedes the design-conversation brief this file replaced (same
filename); the brief's substance survives here, chiefly in its open questions,
which were this interview's agenda.

Flavor: minimal — single-maintainer library, no external users of
`single_header/s2s.hpp` to coordinate with. The consumer is a schema author, so
the declaration surface is the whole product.

Companion: `dev/specs/the-size-axis-cannot-say-read-until-a-byte.md`, completed
and merged (issues 062-068). That feature went first because its incremental
read machinery is what this one also needs, and three of this brief's open
questions were closed during its interview. Its write-side rejection rule, its
shift of `max_bytes` from guard to termination condition, and its distinct
`error_reason` per failure mode are the precedent this document holds itself
to.

## Overview

`vector_of_records` takes its element count from the size axis — a literal, a
sibling field, or a callable — and `read_field` resolves that count, checks the
footprint, calls `resize(n)`, and reads exactly n elements. Some formats supply
no count. A GIF image's data is a run of length-prefixed sub-blocks terminated
by a zero-length sub-block; DNS label lists have the identical shape. The run's
length is known only on reaching the thing that ends it, so today these formats
cannot be expressed at all.

This adds one form, `until_field_equals<"field", value>`, that lets a record
sequence declare what terminates it instead of how many elements it has. It
inverts the read the same way the companion did: with no count there is nothing
to `resize` by, so the loop becomes read-element-then-test and the run's length
becomes an outcome of the read rather than an input to it.

```cpp
using sub_block =
  s2s::struct_field_list<
    s2s::basic_field<"size", u8, 1_B>,
    s2s::vec_field<"data", u8, s2s::len_from_field<"size">>
  >;

using image_data =
  s2s::struct_field_list<
    s2s::basic_field<"lzw_min_code_size", u8, 1_B>,
    s2s::vector_of_records<"blocks", sub_block, s2s::until_field_equals<"size", u8{0}>>
  >;
```

The shape is chosen by the write side. A predicate over the element cannot be
run backwards to produce the thing it accepts, and a whole-element terminator
value is not structural whenever the record holds a `std::vector` or
`std::string` — which is the common case and true of the motivating example. A
field id plus a value is both structural and invertible: on read it is the
test, on write it is a constructor.

## Goals

- **A termination form spelled `until_field_equals<"field", value>`**, accepted
  by `vector_of_records`, that reads elements until one whose named field equals
  the sentinel value.
- **The terminating element is consumed and discarded.** It does not appear in
  the user's vector, so the element count means what the format means. Read is
  read-then-test, so the loop always consumes one element more than it keeps;
  for a degenerate terminator this consumes exactly the bytes the terminator
  occupies and strands nothing.
- **Write synthesises the terminator.** Default-construct an element, set the
  named field to the sentinel, write it after the last real element. Because
  `data`'s length is driven by `"size"`, the GIF case emits exactly one zero
  byte.
- **The sentinel value is a compile-time constant of any structural type the
  named field can hold.** Not restricted to integrals — both witnesses happen to
  be integral, which is precedent, not a rule being imposed. The named field's
  type must be equality-comparable with the value.
- **A compile-time synthesis-soundness check, as a hard error.** Every field of
  the element other than the named one must have a determined width in the
  synthesised terminator: each is either fixed-width, or has its length driven
  — directly or transitively — by the named field. A terminator whose other
  fields have independent widths would emit a full-size element rather than a
  sentinel, so a schema that cannot synthesise correctly must not compile.
  Deliberately not the cheaper positional rule ("must name the element's first
  field"), which rejects a legal schema whose sentinel field follows a
  fixed-width one, and not left unchecked, which reintroduces the corruption
  class this feature exists to avoid. **Testable:** a `test/must_not_compile/`
  case for a violating schema.
- **Combining `until_field_equals` with a size-axis count on the same
  `vector_of_records` is a compile error.** Two contradictory statements about
  what ends the run. **Testable:** a `test/must_not_compile/` case.
- **An empty run is valid.** A terminator in first position yields an empty
  vector, consuming the terminator's bytes. Writing an empty vector emits the
  lone synthesised terminator. A minimum length is a constraint the schema
  author writes, not policy the engine imposes.
- **Testable, write side:** a vector containing an element that matches the
  sentinel fails to write, under its own error reason. Written whole, such a
  vector reads back short — elements after the match are silently lost with no
  error raised anywhere. This is the only failure mode in the feature that would
  otherwise produce a wrong answer rather than an error, and it is the same
  class as the companion's delimiter-in-value rejection.
- **Testable, round trip:** GIF sub-block runs and DNS label lists both
  round-trip byte-identically, including the empty-run case. These are the
  feature's acceptance witnesses, named during the companion's interview.
- **Both a constexpr-stream form and a runtime-stream form**, per the project's
  standing override.
- **Documented:** a `docs/` page for the new form and one `test/doc_examples/`
  program bound into a fenced block, per `AGENTS.md`.

## Non-Goals

- **Runtime-valued sentinels.** No `until_field_equals_field<"size", "sep">`
  form. Rejected on evidence rather than deferred: no format in the scan
  declares its terminator value in its own header — GIF's is a hardcoded zero
  sub-block, DNS's a hardcoded zero length octet. The companion rejected
  runtime-valued delimiters on the same evidence.
- **Stream lookahead — `peek`, `unget`, or a lookahead capability concept.**
  Rejected on merits, and the brief's original reasoning is corrected here: such
  an addition would be **additive, not breaking**, since a capability concept
  plus `if constexpr` dispatch leaves every existing custom stream compiling on
  a fallback path. It is out of scope for the real reasons instead. Lookahead
  buys only that the terminator stays unconsumed, which matters only when the
  terminator is *not* a self-contained degenerate element — and for both
  witnesses it is, so read-then-test consumes exactly the right bytes. Against
  that, it costs a second stream capability with fast and fallback paths in
  both constexpr and runtime form: four configurations, the same testing cost
  that kept `read_until` out of the companion. Note also that `peek` is the
  wrong primitive regardless — `std::istream::peek()` pays the same per-call
  sentry cost as `read`, and peeking K bytes at once has no `std::istream`
  spelling that does not require a seek.
- **Delimited runs of trivials.** `until<>` is accepted on `vec_field` as well
  as `str_field`, so a run of trivials ending at a sentinel value is already
  spelled there. Two ways to say one thing would be worse than one.
- **Multi-condition terminators.** Anything richer than field-equals-value — an
  `lt`-style constraint, several fields at once, a predicate. The library
  already distinguishes constraints the write path can satisfy (`eq{v}`) from
  ones it can only check (`lt{100}`); a terminator must be in the first class,
  because the engine has to produce it.
- **`until_field_equals` on `array_of_records`, optionals, or unions.** An array
  is terminated by its fixed count; an optional and a union have no run to
  terminate.
- *Deferred rather than excluded:* **region-bounded termination.** An enclosing
  known length does not cap the run ahead of `max_bytes`. Same deferral the
  companion made, for the same reason: the read path carries no notion of
  remaining-region bytes today.

## Technical Approach

Hosted C++23, gcc 14+ for constexpr use. Header-only, amalgamated into
`single_header/s2s.hpp`. No new dependencies.

**Where it lands.** A new termination type alongside the size forms in
`include/field_size/field_size.hpp`, classified by its own trait rather than a
widening of an existing one — `is_delimited_size` set that precedent for
exactly this reason. The read inverts the `vector_of_record_field_like`
specialization of `read_field` in `include/field_read/field_reader.hpp`; the
write-side synthesis and rejection land in `include/field_write/`. The
synthesis-soundness check is a compile-time walk over the element field list,
expressed as a named concept per the project's concepts-not-SFINAE override.

**Incremental growth, and a bound that is load-bearing.** With no count there is
no `resize(n)`, so elements are appended one at a time and the footprint is
re-checked after each. The denominator is `n * sizeof(element)` — the in-memory
footprint, not wire bytes — which is the denominator already settled for
bounded vectors of records, and for the same reason: a record carrying
variable-length subfields has no statically known wire size. This differs
deliberately from the companion, whose bound counts the value's bytes. The
bound is `max_bytes<N>` if declared, otherwise the 16 MiB default.

As in the companion, this is a change in kind rather than degree. For every
other field the bound guards against a corrupt declared length; here it is
structural, because a corrupt stream that never yields its sentinel loops until
the ceiling stops it and nothing else will.

**The bound is per run, and the PRD says what that does and does not
guarantee.** Each `vector_of_records` checks its own accumulated footprint
against its own bound, which is how every bound in the library already works
(`bound_in_bytes<T::field_bound>` is per-field) and changes no reader
signatures. The consequence, stated rather than implied: when a
sentinel-terminated run nests inside another, worst-case footprint is the
**product** of the bounds, not the outer bound. A single budget threaded
through the whole read would give the stronger guarantee, at the cost of a
remaining-budget parameter reaching every field reader — the same threading
cost that kept a pushback buffer out of the companion.

**Three outcomes, three reports**, mirroring the companion exactly. Sentinel
element read is success. The stream running dry mid-run is `buffer_exhaustion`,
which describes that fact correctly. The bound being reached with no sentinel
found is a new `sentinel_not_found`: truncated and corrupt are different facts
about a file, and a caller can act differently on them. Reporting either as the
other is technically true and diagnostically useless.

**The write-side rejection gets its own reason**, e.g.
`found_sentinel_in_sequence`, rather than being folded into
`validation_failure` — the schema author has not violated a constraint they
wrote, they have hit a rule of the termination form. It is a per-element check
on write. Both new `error_reason` enumerators are **appended**, keeping existing
values stable, as `found_contradicting_length`, `excessive_length`,
`delimiter_not_found` and `found_delimiter_in_value` already did.

**Failure leaves no partial value, and does not rewind the stream.** A failed
read returns `unexpected` and `struct_cast_*` hands back no struct at all, so a
partially-filled vector is never observable; the stream is left wherever the
failure occurred, as for every other field. On write, a rejection fails at the
offending element with earlier bytes already emitted — no rollback, matching
every other write-side rejection in the library.

**Testing**, per `AGENTS.md`: a `test/schema/` read/write/`_ct` triple for the
new construct, `test/must_not_compile/` cases for the two compile errors, and
one `test/doc_examples/` program bound to its fenced block.

## Open Questions

- Where the write-side sentinel check fires. The whole vector is in hand before
  any of the run is emitted, so it could reject before writing a byte rather
  than at the offending element — the chosen behaviour is "at the offending
  element", but a pre-pass is available if the architect prefers it. The
  companion left the equivalent question (compile-time vs runtime rejection of a
  delimiter-bearing literal) open for the same reason.
- Naming. `until_field_equals<"size", u8{0}>` is the brief's spelling and
  nothing has challenged it; the companion's `until<u8{0}>` is the pair it has
  to read well beside. The project's standing naming sweep
  (`dev/inbox/todos.md`) is deliberately last and will see both.
- How the synthesis-soundness check reports. Knowing *which* field defeats
  synthesis makes the diagnostic actionable; whether the trait can carry that
  through to the `static_assert` message is a design question.
