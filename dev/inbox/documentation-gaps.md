# Documentation gaps — input for the requirements interview

Notes from a working session on 2026-08-09. Not a spec. This is the raw
material a PRD for "documentation" should be written against, so that the
interview settles requirements rather than inventing a nav tree.

The framing that matters: **field descriptors are not read-side features.**
They are the schema language, and it is direction-neutral. Both `struct_cast`
and `stream_cast` are driven by the same declaration. Structuring the docs by
direction is what produced the current asymmetry.

---

## 1. Current state

### The write direction is documented well

`UserGuide.md` "Writing to stream" is the strongest material in the repo:
a compiling worked example, "Derived fields are read-only" with the
rationale for *why* derivation exists, a check-to-`failure_reason` table, and
an explicit write-once/fail-fast contract including what happens on a partial
stream failure. It documents the reasoning, not just the signatures.

Its problem is filing, not quality — see §3.

### The read direction is barely documented at all

This is the inversion worth putting in front of the interview: the older,
complete, more-used direction has the thinner documentation.

`UserGuide.md` opens with a table of contents listing:

```
Reading from stream
    Trivial / Array of trivials / Array of records
    Length prefixed vector of trivials / Length prefixed vector of records
    Const sized strings / Length prefixed strings
    Optionals / Unions / Magic strings / Magic numbers
Using custom stream
Validating field members
```

**None of those sections exist.** Not one of the twelve field kinds has prose.
"Using custom stream" and "Validating field members" have no section either.
The table of contents is a promise the file never keeps, and it has been that
way long enough to be load-bearing in the wrong direction — a reader scrolling
for "Optionals" concludes the library is undocumented rather than that the
heading is missing.

What the read direction actually has:

| | Read | Write |
|---|---|---|
| Worked example in UserGuide | none | yes, compiling |
| Per-field-kind prose | none | n/a (correctly, it's schema) |
| Error table | one sentence, "three ways" | full table, check → reason → `failed_at` |
| Contract/semantics prose | none | derivation model, fail-fast, atomicity unit |
| Signature block | yes | yes |

The entire "Cast" section is twelve lines: two signatures and one paragraph.
The README carries better read examples than the user guide does.

### Two concrete defects

- `UserGuide.md` lines 65–107 have **no opening code fence**. The whole
  field-descriptor alias block renders as run-together prose. A large part of
  "the UserGuide is incomplete" is this: it is not missing, it is unreadable.
- `README.md` line 81 tells the reader to
  `#include "test/constexpr_memstream.hpp"`. The actual path is
  `test/utils/constexpr_memstream.hpp`. The constexpr example cannot be
  copy-pasted as written.

---

## 2. Public vocabulary with no prose anywhere

Everything below is exported through `include/s2s.hpp` and is part of the
supported surface. Nothing in `README.md` or `UserGuide.md` explains any of
it. Several are named exactly once, in the write chapter, while explaining
what is *not* derived — i.e. the only way to discover them is a passing
mention describing what they don't do.

### Type deduction for unions — an undocumented subsystem
`type_switch`, `match_case`, `if_else_ladder`, and the clause vocabulary in
`include/type_deduction/`.

The README's single "Unions" bullet is hiding two distinct mechanisms: a
switch on a discriminant field's value, and a predicate ladder over arbitrary
sibling fields. They have different capabilities and different write-side
behaviour (a `type_switch` discriminant is derived; a ladder is only
verified). This is the largest single gap.

### Presence deduction
`parse_if`, `predicate`, `eval_bool_from_fields`, `always_present`.

`maybe` is documented as a feature name only. How you say *when* a field is
present is undocumented.

### Computed sizes
`len_from_fields`, `size_from_fields`, `eval_size_from_fields`, `size_choices`,
`compute`.

`len_from_field` (singular) appears in examples. The plural,
callable-driven form — and the fact that it is verified rather than derived on
write — appears only in the write chapter's list of exceptions.

### The constraint DSL
`eq`, `neq`, `lt`, `gt`, `lte`, `gte`, `any_of`, `range`,
`is_in_open_range`, `is_in_closed_range`, `no_constraint`.

Eleven comparison constructs in
`include/field_validation/field_value_constraints.hpp`. The docs name two:
`eq` (implicitly, via magic fields) and `no_constraint` (only as a default in
a signature). "Validation of fields in place" is a README bullet with an
entire expression vocabulary behind it.

### Custom streams
`input_stream_like`, `output_stream_like`, `readable`, `writeable`,
`read_trait`, `write_trait`, `constexpr_read_trait`, `constexpr_write_trait`,
`identified_as_constexpr_stream`, `constexpr_stream`.

"Pluggable interfaces working with custom streams" is a headline README
feature. There is no documentation of what a stream must provide. The only
reference implementation is `test/utils/constexpr_memstream.hpp`, reachable
only by reading the test tree — and pointed at by a broken path in the README.

Note this is also the constexpr story: deriving from `constexpr_stream` is
what unlocks compile-time parsing, and that link is undocumented.

### Descriptors absent from every list
`struct_field`, `c_arr_field`, `c_str_field`.

### The constexpr story — two claims collapsed into one

`README.md` offers a single bullet, "constexpr! as much as possible", and one
constexpr example. That conflates two distinct properties with different
audiences, different requirements, and very different reach:

| | Parser generated at compile time | Parsing performed at compile time |
|---|---|---|
| What it means | the schema unrolls into straight-line code; no runtime descriptor walk, no interpretation, no dispatch | the whole parse runs in constant evaluation and the result is a `constexpr` value |
| Who gets it | **everyone, always**, including a plain `std::ifstream` at runtime | only schemas with no allocating fields, only with a `constexpr_stream` |
| Requires | nothing beyond using the library | derive from `constexpr_stream`; no `std::vector` / `std::string` fields |
| Why you'd care | this is the reason to use s2s over a hand-rolled parser | compile-time constants, static validation of embedded blobs |

The first is the load-bearing claim and it is currently invisible. The second
is the one the README leads with, and it reads as a precondition rather than a
bonus — a reader plausibly concludes the library's value is gated behind
`static_assert`-able parses and stops reading when their schema has a
`std::string` in it.

This is a pitch problem before it is a documentation problem, and it likely
changes the README's opening paragraph rather than just a heading. Worth
settling in the interview: which of the two leads.

---

## 3. A proposed spine (for the interview to accept or reject)

Every plain descriptor is the same primitive with axes pinned:

```
field<id, T, size, constraint>
```

- `basic_field` pins nothing
- `fixed_array_field` pins `size` to `fixed<N * sizeof(T)>`
- `magic_string` pins `constraint` to `eq{expected}` and the size to match
- `array_of_records` pins `size` to `size_dont_care`

On top sit exactly two combinators, which wrap a descriptor rather than being
one:

- `maybe<base_field, present_only_if>`
- `variance<id, type_deducer>`

Documenting four axes plus two combinators documents the schema language. The
twelve descriptors then collapse into one table — name, what it pins, what is
left for you — rather than twelve sections.

**The volume is not in the descriptors. It is in the size axis.** `fixed`,
`len_from_field`, `len_from_fields`, `size_dont_care`, `size_choices` are
where direction-dependence genuinely lives, and where the two directions
actually differ:

| Size kind | On read | On write |
|---|---|---|
| `fixed<N>` | reads N bytes | writes N bytes |
| `len_from_field<"n">` | `n` drives the read length | **derived** from the container's `size()`; the field is read-only |
| `len_from_fields<f, ...>` | callable drives the read length | **verified** against the container; disagreement is `found_contradicting_length` |
| `size_dont_care` | field computes its own extent | same |

Most of the existing write chapter migrates into this table and gets *shorter*
in the move. What remains under "Writing" is the call signature, the error
reasons, and the fail-fast contract.

Suggested shape, subject to the interview:

1. **Schema** — the four axes, the descriptor table, the two combinators, the
   size axis in full, type deduction, presence deduction.
2. **Constraints and validation** — the eleven-construct comparison
   vocabulary, and when it fires: in place during the read, and before the
   first byte of the field during the write. Two halves, one section: what you
   can express, and when it is checked.
3. **Reading** — signatures, one worked example, errors.
4. **Writing** — signatures, one worked example, errors, fail-fast contract.
5. **Streams** — what a custom stream must satisfy (`read_trait` /
   `write_trait`, or the constexpr pair), with a real reference
   implementation rather than a pointer into the test tree.
6. **Compile time** — the two claims separated: what everyone gets for free,
   and what the `constexpr_stream` path additionally buys. Belongs next to
   Streams, since deriving from `constexpr_stream` is the mechanism.
7. **Reference** — the alias list, correctly fenced.

---

## 4. Open questions the interview should settle

1. **Is `compute` / `predicate` a user-facing extension point, or an
   implementation detail that leaked into `api/`?** This decides whether it
   gets a page or gets hidden. Nothing in the code states the intent, and it
   is a product call rather than a documentation one.

2. **What is the supported surface?** Related to the above. `include/lib/`
   (`static_vector`, `fixed_string`, `static_map`) is reachable from the
   single header. Are those documented, mentioned, or explicitly excluded?

3. **README versus user guide — what is the split?** Currently the README
   duplicates the user guide's struct and descriptor sections almost verbatim
   under "Brief Tour", and the two have already drifted. Either the README
   shrinks to a pitch plus one example, or the duplication gets a rule.

4. **Does the write direction get an equal-prominence README example?** The
   README has two read examples and zero write examples, which is what
   started this session. A round-trip example would cover both and is already
   written — `test/single_header/roundtrip.cpp`.

5. **Site generator: needed at all?** The material above is roughly five
   pages. mkdocs-material would be the choice if one is wanted (no Rust here,
   so mdBook buys nothing but familiarity for the wrong audience). The
   recommendation from this session was to defer the decision until the schema
   section is actually written and its size is known — picking the container
   before knowing the contents is how nav trees fill with stubs.

---

## 5. Constraint worth carrying into the PRD

`test/single_header/doc_example.cpp` compiles and runs the README example
against the shipped single header, as a registered test. That is the reason
the examples are not rotting.

Any restructure should keep every non-trivial example backed by a file under
`test/single_header/`. A docs site makes it considerably easier to accumulate
prose examples that nothing compiles — and the broken include path in §1 shows
what happens to the examples that aren't covered.
