# Reading two fields costs a whole record

PRD. Supersedes the design-conversation brief this file replaced (same
filename, 2026-08-30). The brief's substance survives here: its
"Considerations the discussion surfaced" are carried into Overview and
Technical Approach, its "Directions rejected" are restated under Non-Goals
and not re-litigated, and its "Questions still open" were this interview's
agenda and are all answered below.

Flavor: standard — s2s is a header-only library with external consumers
(anyone including `single_header/s2s.hpp`).

## Overview

`struct_cast` parses everything: every field is read off the stream,
converted, byte-swapped if needed and stored before the caller sees any of
it. That is right for a caller who wants the whole record, and wrong for
scanning — walking a million pcap packet headers, ELF section headers or MFT
entries to filter on two fields pays for every field of every record to use
two of them.

This feature adds a second, separate public API: a read-only, lazy **view**
over one record of a **fixed-layout** schema held in contiguous memory. It
decodes a field when that field is asked for and never otherwise. It needs
something the library has never computed — where field *k* begins — and that
offset exists as a compile-time constant only when every field before it has
a compile-time width. So the schema language is restricted, not the
implementation made cleverer: a view accepts a strict subset of schemas, and
that subset is a property checked at compile time, not a limitation to be
engineered around later.

A view defers the copy rather than avoiding it. Each field it reads is
copied into a correctly typed local and `bit_cast`, costing what the parser
costs for that field; a byte-swapped field cannot be returned by reference
even in principle. The saving is the untouched fields and nothing else —
real for a scan, close to nothing for a caller reading one record in full.

## Goals

**Shape of the API**

- A view is created through its own entry point pair mirroring the parser's,
  one little-endian and one big-endian, separate from `struct_cast` (names
  left to design). Byte order is fixed at the call.
- Input is a read-only `std::span` whose element type is `std::byte`,
  `unsigned char` or `char`, of static or dynamic extent. Bytes past the
  record's width are ignored, so a view can sit at the front of a larger
  buffer.
- Creation returns `std::expected<view, cast_error>`. It:
  1. checks the span's length first — a static extent shorter than the
     record is a compile error; a dynamic extent shorter than the record
     fails with `buffer_exhaustion` before any constraint is evaluated;
  2. then evaluates every **constrained** field at every depth, in wire
     order, including every element of every `array_of_records` — and
     nothing else. Unconstrained fields are not decoded.
- Reads are `view["name"_f]` and return a **`const` value**, so that an
  assignment through a view (`view["arr"_f] = {...}` on a class-type value)
  fails to compile instead of silently doing nothing.
  - A trivial field returns its value; a fixed array of trivials returns the
    whole `std::array`; `fixed_string_field` and `magic_string` return
    `fixed_string<N>`.
  - `c_arr_field` and `c_str_field` are accepted and return the equivalent
    `std::array<T, N>` / `std::array<char, N + 1>` — a C array cannot be
    returned by value. The type difference from the parsed struct is
    documented.
  - A `struct_field` returns a **sub-view**; an `array_of_records` is indexed
    with `[i]` and each element is a sub-view. Laziness holds all the way
    down: `v["hdr"_f]["len"_f]`, `v["entries"_f][i]["flags"_f]`.
- A view is a cheap, copyable, non-owning value over the caller's bytes.
- The record's wire width is exposed as a public compile-time constant, so a
  caller can step through a run of records itself (a fixed stride for ELF
  section headers; header width plus `incl_len` for pcap).

**Equivalence with the parser** — the core acceptance criterion

- For every schema a view accepts, a view's value for each field equals what
  `struct_cast_le`/`_be` (same byte order) produces from the same bytes —
  including narrow `N_B` integer fields, floating-point fields, foreign byte
  order, and `c_arr_field`/`c_str_field` compared elementwise.
- For a span at least as long as the record, view creation fails if and only
  if `struct_cast` with the same byte order fails on those bytes, with the
  same `cast_error` — same `failure_reason`, same `failed_at` (including the
  existing limitation that a nested violation names the outermost field).

**The fixed-layout rule**

- Accepted: `basic_field`, `fixed_array_field`, `c_arr_field`, `c_str_field`,
  `fixed_string_field`, `magic_byte_array`, `magic_string`, `magic_number`,
  and `struct_field` / `array_of_records` whose nested schema is itself
  accepted, recursively.
- Rejected: `vec_field`, `str_field`, `vector_of_records` (their size axis is
  always run-time or discovered), every `maybe`, every `variance`, and any
  schema containing `announces_byte_order` at any depth.
- The rule is a **named concept**, built from named sub-concepts (e.g. every
  field has a compile-time width; no optional or union field; no announcing
  record — names left to design), and it **constrains the view entry point
  directly**. The concept must evaluate to `false` for a rejected schema,
  never hard-error, so it contains no `static_assert`.
- It is tested positively and negatively as a trait in the `ut` suite
  (`static_assert(concept<accepted>)`, `static_assert(!concept<rejected>)`),
  and via `test/must_not_compile/` cases matched (`add_rejected_case_matching`)
  on the failing sub-concept's name: one case per rejected descriptor kind,
  plus one with the offending field nested a record deep to prove the check
  descends.

**Cost**

- Creation decodes only constrained fields; a read decodes only the field
  asked for and the sub-view path to it; nothing allocates, in either.
- Every field offset is a compile-time constant; no offset is computed at run
  time on access.
- Verified structurally, not by timing: tests mutate the bytes of fields
  that are not read and are unconstrained, and observe that creation and the
  reads that were made are unaffected.

**Alignment safety**

- A view never forms a typed pointer or reference into the buffer. Every
  read copies the field's bytes into a correctly typed local — `memcpy`-like
  at run time, `bit_cast` of a byte array during constant evaluation, the
  split `read_native_impl` already runs. Sub-views hold a byte pointer and an
  offset, never a typed record pointer.
- Consequently there is no alignment requirement on the buffer's address or
  on any field offset, and no access traps on a strict-alignment target.
- Tests cover a view over a buffer starting at an odd address, and a schema
  whose fields sit at misaligned offsets (e.g. `u8` then `u32` then `u16`),
  reading correct values in both byte orders.
- Correct values alone do not prove alignment safety: x86 does not trap on a
  misaligned typed load, so a regression to a typed pointer would still pass
  there. The view's run-time tests — at minimum the odd-address and
  misaligned-offset cases, ideally the whole view suite — therefore also run
  under UBSan with alignment checking (`-fsanitize=alignment` at minimum,
  full `-fsanitize=undefined` preferred), and a sanitizer report fails the
  test.
- That sanitized run is part of the normal test suite, runnable locally and
  not only in CI: either on by default in the developer build, or reached by
  one documented command (a CMake option or preset). The repo has neither
  today — no `CMakePresets.json`, no sanitizer flags in `CMakeLists.txt`, and
  CI (`.github/workflows/docs.yml`) builds only the docs — so the mechanism is
  new and is the design's to choose. It applies to the gcc and clang
  toolchains; MSVC has no UBSan equivalent and is exempt.

**Constant evaluation and test tiers**

- Creation and reads are usable during constant evaluation. Coverage follows
  `dev/specs/compile-time-test-tier.md`: `ut` suites registered with
  `add_ut_test` (compile-time and run-time), plus GoogleTest cases over a
  buffer read from a real file for the run-time path.

**Documentation**

- The docs state plainly that a view is not zero-copy: it saves the fields
  that are not touched and nothing else, and buys little for a caller who
  reads every field.
- **The unsafe contract** is stated wherever the view API is documented —
  each entry point and accessor's reference entry and the guide page — not
  once in a caveat section:
  - **Lifetime is wholly the caller's.** The view borrows; nothing in the
    library or the language checks that the bytes outlive it, and using a
    view after its bytes are gone is undefined behaviour.
  - **Bounds are checked once, at creation**: at compile time for a static
    extent, against the span's length for a dynamic one. A span constructed
    from a wrong pointer or a wrong size cannot be detected.
  - **`array_of_records` indexing is unchecked.** `[i]` with `i >= N` is a
    documented precondition violation, like `std::array::operator[]`. Because
    trailing bytes are permitted, an out-of-range index can land inside the
    span and silently return garbage rather than crash; the docs say so and
    mark it as an unchecked spot.
- Every program shown follows the project's doc-example binding rule
  (`test/doc_examples/`, `doc_examples_match`).

## Non-Goals

- **Writing through a view.** Read-only. `stream_cast` already covers the
  write direction, and a mutable view would double the surface to test.
- **Variable-length fields, and a run-time offset table to recover them**
  (brief, rejected). One `len_from_field` and every later offset stops being
  a constant; recovering them means walking the record and allocating, on
  the path whose purpose is avoiding work.
- **`maybe` and `variance`, including a `variance` whose alternatives all
  share one width.** A deliberate exclusion that could be revisited: the
  width is constant but the alternative is data-dependent, and decoding a
  variant per access cuts against laziness. One simple rule — every field's
  width and type known at compile time — is preferred for now.
- **Schemas with an announcing record, and a caller-supplied run-time byte
  order.** Either would put a run-time branch on every access. The run-time
  order was deliberately deferred by
  `dev/specs/byte-order-is-data-not-a-template-parameter.md` and is not
  decided here. A pcap scanner reads the file header once and picks the
  `_le` or `_be` view outside its loop.
- **Views backed by a stream** (brief, rejected). `ifstream` is not
  contiguous memory; the requirement was never a stream of known size,
  since an all-fixed schema's width is already a compile-time constant.
- **`reinterpret_cast` at a computed offset** (brief, rejected). Undefined
  for unaligned access and illegal during constant evaluation.
- **Returning references into the buffer** (brief, rejected). Impossible for
  a byte-swapped field and unsound for an unaligned one.
- **Restricting views to aligned schemas.** A schema describes the wire, and
  wire formats are packed; requiring aligned offsets would reject the very
  pcap/ELF/MFT-style formats that motivate this, and would need wire padding
  the schema language does not express. Copy-and-`bit_cast` already makes
  misaligned offsets safe, so a restriction would buy nothing.
- **Iterating runs of records** — a range or generator of views. That belongs
  to `dev/inbox/a-file-of-records-must-be-parsed-all-at-once.md`, which asks
  whether it composes with views: a view is the natural element type, and
  that brief should be designed knowing views exist. This PRD only exposes
  the record width.
- **Lifetime checking** of the borrowed bytes.
- **A benchmark or timing gate.** Acceptance is the structural rule above.
- **Any change to `cast_error`.** It stays `{failure_reason, failed_at}`; the
  nested-failure naming limitation in `dev/inbox/todos.md` applies unchanged.
- **Bitfields** (`dev/inbox/a-packed-byte-is-just-a-byte.md`). Whether a
  bitfield container is admissible in a view is that brief's call when it
  lands.

## User Stories

- As a tool author filtering a million pcap packet headers on two fields, I
  want to view each header in place and read only those two, so that the
  scan does not pay to decode fields it never looks at.
- As a schema author, I want to reuse the schema I already parse with
  `struct_cast` to create a view, and to get a compile error naming which
  part of the fixed-layout rule it breaks if it cannot be viewed, so that I
  never learn about an unviewable schema at run time.
- As a caller holding a `char` buffer from a file, I want to create a view
  directly over it at whatever offset the record sits, so that I do not cast
  or copy the buffer first or worry about its alignment.
- As a reader of the docs, I want the borrowing, bounds and not-zero-copy
  caveats stated where the API is described, so that I do not discover them
  by debugging a dangling view or an unexpected cost.

## Technical Approach

- C++23, header-only under `include/`, amalgamated by `scripts/amalgam.py`.
  Project conventions apply: trailing return types, `[[nodiscard]]` on
  public entry points, named concepts, include guards with relative paths,
  `std::expected` errors.
- **Offsets.** A compile-time prefix sum over the declared widths,
  recursing into nested records and multiplying through `array_of_records`.
  Whether it is a constexpr offset array over a schema value or recomputed
  by template recursion per accessed field is a design decision; the brief's
  observation that this is the feature that most benefits from value
  lowering is recorded for the designer. The work is **not gated** on value
  lowering — size-axis value lowering has landed (043–045), a full value IR
  has not, and nothing here needs it.
- **Field reads.** Copy the field's bytes from `base + offset` into a local
  and `bit_cast`, reusing `read_native_impl`'s constexpr/run-time split
  driven by an offset instead of a cursor; byte-swap foreign scalars and
  elements as `read_foreign_scalar`/`read_foreign_buffer` do. Narrow `N_B`
  fields must reproduce the parser's placement of the bytes within the wider
  type exactly.
- **Constraint evaluation at creation.** Reuse the existing constraint
  machinery from `field_value_constraints.hpp` over the constrained fields
  only, producing the same `cast_error` the parser would.
- **The fixed-layout concept.** Departs on purpose from the byte-order
  precedent — `fixed_order_schema`/`self_announcing_schema` in
  `include/field_list/announcing_record.hpp`, concepts backed by a struct of
  `static_assert`s that hard-error rather than evaluate to `false`, and which
  therefore have no negative trait tests. Here the concept must be pure so
  it can be tested as a trait; diagnostic readability comes from the
  sub-concept names appearing in the compiler's unsatisfied-constraint
  trace. The design should note this divergence and why.
- **Span element types.** All three accepted types are byte-sized, have
  alignment 1, and may alias any object; the input never needs to be
  reinterpreted as anything wider.
- **Tests.**
  - `test/schema/`: a `<feature>_ct.cpp` `ut` suite and a GoogleTest
    counterpart exercising the equivalence criterion against `struct_cast`
    in both byte orders, for each accepted descriptor kind, nested records
    and arrays of records; error parity on constraint failures and short
    spans; misaligned buffer and misaligned offsets; mutation of untouched
    fields.
  - `test/internals/` (or alongside): positive and negative trait checks of
    the concept and each sub-concept.
  - `test/must_not_compile/`: one matched case per rejected kind, one
    nested, one static-extent span shorter than the record, and one
    assignment through a view accessor.
  - A UBSan-instrumented build (gcc/clang) running at least the view's
    run-time cases — the GoogleTest suite and the `ut` run-time entries;
    the `*_compile_time` entries only drive `cmake --build` and gain nothing
    from it — with sanitizer reports made fatal (e.g.
    `-fno-sanitize-recover`) so CTest sees a failure. Whether it is a
    default, a CMake option, or a preset is a design decision; one
    documented local command is binding.
- **Docs.** A guide page and reference entries under `docs/`, listed in
  `docs/mkdocs.yml`, carrying the not-zero-copy statement and the unsafe
  contract at each point of use.

## Success Metrics

- For each accepted descriptor kind, nested and in arrays of records, in
  both byte orders, every field read through a view equals the same field
  from `struct_cast` over the same bytes — at compile time and at run time.
- For bytes that violate a constraint, view creation and `struct_cast`
  return equal `cast_error`s.
- A dynamic span one byte short fails creation with `buffer_exhaustion`; a
  static span one byte short does not compile.
- A view over a buffer at an odd address, and over a schema with misaligned
  field offsets, reads correct values, and does so with no UBSan report when
  the suite is run under the sanitized configuration — locally, via its one
  documented command.
- Changing the bytes of an unconstrained field that is never read changes
  neither creation's result nor any value read.
- The concept is `true` for every accepted schema used in the tests and
  `false` (not a hard error) for each rejected kind; each matched
  `must_not_compile` case fails for its intended reason.
- `ctest` is green, including `doc_examples_match` and
  `docs_nav_lists_every_page`.

## Future Directions (non-binding)

- Once the library's baseline is C++26, the unchecked `array_of_records`
  index — and possibly other documented preconditions in the unsafe contract
  — should be expressed with standard library hardening and contract
  preconditions (`pre`) rather than prose alone. Recorded so it is not lost;
  not a requirement of this work.

## Open Questions

None outstanding from the interview. The brief's agenda was resolved as:

- Eager or lazy validation: eager over constrained fields only, at creation.
- Mutable view: no (Non-Goals).
- Same `["name"_f]` accessor: yes, returning a `const` value.
- Nested records and arrays of records: sub-views.
- `maybe` / same-width `variance`: rejected, deliberately.
- Enforcement and diagnostic: a pure named concept of named sub-concepts on
  the entry point, trait-testable both ways.
- Offset table as a value or per-access: left to design; not gated on value
  lowering.

Implementation-level decisions left to the architect: entry-point and
concept names; the offset mechanism; the sub-view's exact type; and what
`failed_at` holds for a short-span `buffer_exhaustion`, where the parser
might instead have reported a constraint failure on an earlier field.
