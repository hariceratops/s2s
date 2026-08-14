# Bound variable-sized field allocation against a corrupt wire length

PRD. Supersedes the design-conversation brief this file replaced (same
filename); the brief's substance survives here, chiefly in its Open
Questions section, which was this interview's starting agenda.

Revised after freezing, to make safety on by default rather than opt-in —
see Goals. The rest of the interview's conclusions are unchanged.
Flavor: minimal — single-maintainer library, no external users of
`single_header/s2s.hpp` to coordinate with.

## Overview

A stream produced by another tool — corrupt, truncated, or hostile — can
carry a variable-sized field's length as a corrupt value (a `len_from_field`
`u32`, say) and today that length is handed straight to `resize` with
nothing between the stream and the allocation. A `vec_field`/`str_field`
allocates up to 4 GB; `vector_of_records` allocates up to 4 G elements,
before a single byte of payload is read. This work bounds every such
allocation: a build-time default ceiling that applies wherever the schema
declares nothing, and a per-field `max_bytes<N>` that overrides it. A length
over the ceiling is rejected as a recoverable `cast_error` before the
allocation happens. It also fixes an adjacent integer overflow in the same
byte-count computation.

## Goals

- A variable-sized field whose wire length, converted to bytes
  (`count * sizeof(element)`), exceeds a schema-declared `max_bytes<N>` is
  rejected with a `cast_error` before any allocation proportional to that
  length — never a `bad_alloc` or an OOM kill.
- `max_bytes<N>` is a new trailing option in the order-independent pack
  `field_descriptors.hpp` already classifies (`size_option_like` /
  `constraint_option_like`), composing with a size and a constraint option in
  any order:
  `s2s::vec_field<"payload", u8, s2s::len_from_field<"n">, s2s::max_bytes<4096>>`.
  It is declared on the *container* field, never on the length field —
  a single length field can size multiple containers of different element
  widths (see `fanout` in `test/schema/size_axis_read_ct.cpp`, one `"len"`
  sizing both a `vec<u16>` and a `vec<u32>`), a `size_from_fields` length has
  no length field to annotate at all, and the thing being bounded — the
  allocation — belongs to the container, which is also what lets the bound
  be denominated in bytes (what actually matters for memory) rather than
  elements.
- The bound is denominated in bytes uniformly across `vec_field`,
  `str_field`, and `vector_of_records` alike, as `count * sizeof(element)` —
  including `vector_of_records`, where the memory footprint (not the wire
  size, which is not statically known when records carry variable-length
  subfields) is the correct and sufficient denominator. No special case is
  needed for `vector_of_records`.
- `max_bytes<N>` is inclusive: a byte count equal to `N` is accepted, `N + 1`
  is rejected.
- The read path gains a new `error_reason` enumerator, appended the way
  `found_contradicting_length` was (existing enumerator values unchanged),
  naming this rejection distinctly from `buffer_exhaustion` — the reason
  code is the entire enrichment; see Non-Goals on `cast_error`'s shape.
- The `len_to_read * sizeof(element)` overflow at `read_impl.hpp:60` is fixed
  unconditionally, as a correctness bug independent of `max_bytes` — a
  wrapped product today resizes for N elements while reading fewer bytes,
  leaving the tail silently uninitialized. The fix is a checked (or
  saturating) multiply, and it is the same product the `max_bytes` check
  needs, so the two land together.
- `max_bytes` on a fixed-size field is rejected at compile time by the pack
  classifier's concept mechanism — the same mechanism that already rejects
  an unrecognized pack entry, not a bolted-on `static_assert`. A fixed
  size's allocation is compile-time known and cannot be tainted by wire
  input, so accepting the option there would let a schema author believe
  they had bounded something they had not. Covered by a new
  `test/must_not_compile/` case alongside the four already registered across
  `derived_field_assignment.cpp`, `hidden_length_target.cpp` and
  `option_pack_misuse.cpp`.
- The check applies at both places that currently resize from an unvalidated
  wire length — `read_native`'s `variable_sized_buffer_like` overload
  (`read_impl.hpp`, `vec_field`/`str_field`) and the `vector_of_records` path
  (`field_reader.hpp:153`) — with no allocation proportional to the
  unvalidated length occurring at either site. Whether both route through
  one shared helper or keep separate checks is a design-phase decision, not
  pinned here.
- Covered under the project's compile-time test tier: `constexpr_memstream`
  knows its own extent, so the reject-before-allocate path is testable
  without a real file, under both `add_ut_test`'s compile-time and run-time
  modes, per `dev/specs/compile-time-test-tier.md`.
- **Safety is on by default.** A field that declares no `max_bytes` gets
  `default_max_bytes` — 16 MiB — so a schema written without thinking about any
  of this is still protected.

  This revises the opposite decision recorded earlier in this document's own
  history, and the reasoning is the failure mode rather than the number. "Too
  low a default" fails *closed*: an oversized legitimate field is rejected
  loudly, `failed_at` names it, and the fix is one declaration in the schema.
  "No default" fails *open*, silently, in precisely the case nobody thought
  about. A wrong default that announces itself beats no default that does not,
  so "we would have to pick a number" was never the blocker it looked like. It
  costs nothing at run time: the ceiling is a compile-time constant and the
  gate is one comparison against it, so a default is a different constant in
  the same comparison rather than an extra branch.

  16 MiB is a judgement call, recorded so it can be argued with rather than
  inherited: far above what fields in real binary formats carry, far below
  what a corrupt `u32` can claim, and survivable as an accidental allocation
  on any hosted target.
- `S2S_DEFAULT_MAX_BYTES` is the single global knob. Raising it raises every
  default; setting it to `SIZE_MAX` turns the defaults off wholesale, which is
  the escape hatch instead of a second on/off mechanism to build and test.
  **It cannot reach a declared `max_bytes`** — a default is the library
  guessing, a declaration is the author's intent, and no build setting
  discards the latter.
- The default, how to move it, and the fact that a declared bound survives the
  global knob are discoverable in the documentation, not only implicit in the
  option's reference entry.

## Non-Goals

- **No seek / remaining-extent bound.** A declared `max_bytes` bounds the
  allocation on its own; a stream-derived bound would add a new constraint
  on `input_stream_like` that every custom stream must satisfy (a breaking
  change), two independent bound sources with two error stories, and
  protection that varies silently by stream type — all to partially soften a
  default chosen deliberately — at the time, an opt-in ceiling. Seeking may be worth
  adding later, but for *parsing* reasons rather than bounds — offset-
  indirection formats, back-patching a checksum on the write path, trailing
  structures located from the end, skipping, lazy reads, possibly coupled
  with coroutines. A future spec should pick that up on its own merits and
  not inherit this one's framing.
- **`cast_error` gains no new member.** No offending length, no bound value,
  no record of which check tripped. It stays exactly
  `{failure_reason, failed_at}`. Reasoning: `cast_error` returns by value
  from every read, and a per-reason payload means either a fatter struct or
  a variant on a hot path, paid by every caller for a diagnostic few need.
  **Accepted gap, stated explicitly rather than left implicit:** a caller
  learns *that* a declared bound rejected a length and *which field*
  (`failed_at`), but not the length itself. The source brief argued a failed
  parse's only remaining information is on the error path; this PRD narrows
  that promise to reason-plus-field. Error codes stay lean now and can be
  enriched later if a concrete need appears. (This is the same enrichment
  `dev/specs/schema-api-verbosity.md` deliberately deferred to this work;
  it stays deferred.)
- **Write path untouched.** `field_writer` writes `value.size()` from a
  container the caller already owns; it never resizes from a length read off
  a wire, so there is nothing untrusted to bound in that direction.
- **No template parameter on `struct_cast_le`/`_be`.** The bound is declared
  per field in the schema, or comes from the build-time default; it is never a
  property of a particular cast call.
- **No incremental resize-and-fill loop.** Rejected as a bounding mechanism:
  it discards the wire-carried length (real protocol information) in favor
  of "however much arrived," and it pays repeated reallocation cost on every
  large *legitimate* field to defend against the rare corrupt one.

## Technical Approach

- Header-only C++23, under `include/`, following the project's constexpr
  dual-path convention (`identified_as_constexpr_stream` and runtime
  `readable` overloads both covered).
- New pack-option value/type for `max_bytes<N>` (an NTTP-carrying value,
  analogous to `len_from_field<id>` / `byte_count`), living alongside the
  existing size vocabulary in `field_size.hpp` or `field_descriptors.hpp`.
- New classifier concept (e.g. alongside `size_option_like` /
  `constraint_option_like`) recognizing a `max_bytes` entry in the trailing
  option pack; `pack_options` gains a count assertion (at most one bound
  option) the same way it does for size and constraint; `resolved_options`
  gains a resolved bound accessor threaded through `vec_field`, `str_field`,
  and `vector_of_records`' `field<...>` instantiation so the read path can
  see it.
- Compile-time rejection of `max_bytes` paired with a fixed-size field is
  expressed as a concept/`requires`-clause condition in the same
  classification machinery that already rejects an unrecognized pack entry.
- A checked (or saturating) multiply for `count * sizeof(element)` is shared
  between the overflow fix and the `max_bytes` comparison — one product
  computed once, used for both the correctness fix and the bound check.
- New `error_reason` enumerator appended after `found_contradicting_length`
  (name left to design).
- Both current unchecked-`resize` call sites — `read_native`'s
  `variable_sized_buffer_like` overload (`read_impl.hpp` ~line 51) and the
  `vector_of_records` resize (`field_reader.hpp` ~line 153) — gain the
  reject-before-allocate check; shared-helper vs. per-site duplication is a
  design-phase call.
- Test additions: a `test/must_not_compile/` case for `max_bytes` on a
  fixed-size field; `constexpr_memstream`-backed `ut` cases (compile-time
  and run-time, per the project's three-way build) exercising below-bound
  (accepted), at-bound (accepted, inclusive), and over-bound (rejected)
  lengths for `vec_field`, `str_field`, and `vector_of_records`; matching
  GoogleTest coverage for the runtime stream path per the project's
  "new stream-touching code needs both forms" convention.
- Documentation: the schema reference gains `max_bytes`; because its
  default is security-relevant, it needs a discoverable
  callout beyond the reference entry — exact placement left to the design/
  doc-authoring phase, but the requirement (must be discoverable, not only
  implicit) is binding.

## Open Questions

- Naming: the new `error_reason` enumerator and the `max_bytes` option
  type/concept names are left to implementation.
- Whether `read_native` and the `vector_of_records` path share one bound-
  checking helper or keep two independent checks — left to the design
  phase; the PRD only requires that no allocation proportional to an
  unvalidated wire length occurs at either site.
- Exact placement of the allocation-limits documentation — left to the
  design/doc-authoring phase; only its discoverability is binding here.
