# Design: Struct-to-Stream Serialization (Write Direction)

Consumes `specs/struct-to-stream-write.md` and `issues/001`–`issues/011`.
Mirrors the existing read path (`include/field_read/`, `include/cast/`,
`include/api/`). Conventions per `AGENTS.md`: `std::expected` errors,
trailing return types, dual constexpr/runtime stream paths, named concepts,
`#ifndef` guards with relative includes, `include/lib/` vocabulary types.

---

## 1. Header layout (spec open question; issue 001)

### New files

| Path | Contents |
|---|---|
| `include/stream/byte_order.hpp` | `cast_endianness`, `deduce_byte_order<endianness>()` — **moved** out of `field_read/read_impl.hpp` |
| `include/field_write/write_impl.hpp` | `write_native_impl` (constexpr + runtime overloads), `write_native`, `write_foreign_scalar`, `write_foreign_buffer`, `write_impl<endianness>` |
| `include/field_write/derived_value.hpp` | the derivation-obligation model: `derivation_obligation`, `derive_value`, `verify_obligations`, width-fit check |
| `include/field_write/field_writer.hpp` | `write_field<T, F>` per-field-kind specializations, `write_variant_helper`, optional/union verification |
| `include/cast/struct_write_impl.hpp` | `struct_write_impl<F, stream, endianness>` — the left-to-right fold over the field pack |
| `include/api/struct_write.hpp` | public `struct_write_le` / `struct_write_be` |

`field_write/` is the deliberate mirror of `field_read/`: same two-file split
(low-level endian-aware stream primitives + field-kind dispatch), same
naming, plus one file for the write-only derivation logic that has no read
counterpart. The whole-list driver goes in `cast/` next to
`struct_cast_impl.hpp` because it is the same layer, not a new one; the
public entry point goes in `api/` next to `struct_cast.hpp`.

### Modified files

| Path | Change | Why |
|---|---|---|
| `include/field_read/read_impl.hpp` | delete the `output_stream` scaffold; move `cast_endianness`/`deduce_byte_order` to `stream/byte_order.hpp` and include it | the scaffold is superseded (§1.1); the byte-order helpers are direction-neutral and a `field_write → field_read` include would be backwards |
| `include/stream/stream_traits.hpp` | `output_stream_like` becomes `(identified_as_constexpr_stream<T> \|\| writeable<T>) && convertible_to_bool<T>` | today a constexpr stream cannot satisfy `output_stream_like` at all, so the public constexpr write path would be unconstrainable. Mirrors `input_stream_like` exactly. Keeps the spec's non-goal: no combined in/out concept, only a shared marker base |
| `include/error/cast_error.hpp` | move `operator\|(const cast_result&, callable)` here from `cast/struct_cast_impl.hpp`, and `operator\|(const rw_result&, callable)` here from `field_read/field_reader.hpp` | **required, not cosmetic**: `struct_write_impl` needs the same pipeline operators, and `scripts/amalgam.py` concatenates all headers into one TU — a second definition would be a hard error |
| `include/lib/memory/address_manip.hpp` | `as_byte_buffer` takes `const T&`; add `const char* byte_addressof(const T&)` overloads constrained on `output_stream_like` | the write source is `const`. `as_byte_buffer` is already direction-neutral (constrained on `identified_as_constexpr_stream`), so one signature change serves both directions instead of a duplicate |
| `include/field_list/field_list_metadata.hpp` | add `extract_unconditional_len_sources`, `extract_conditional_len_sources`, `extract_switch_discriminants`, the resulting `derived_field_ids`, and `is_derived_field<metadata>(sv)` | §4 |
| `include/field_list/field_list.hpp` | the constrained `operator[]` overload set | §4.3 |
| `include/s2s.hpp` | `#include "api/struct_write.hpp"` | ship it |
| `test/utils/constexpr_memstream.hpp`, `test/utils/s2s_test_utils.hpp` | writable `memstream`, round-trip macros | test-side only |

### 1.1 The `output_stream` scaffold — removed, not completed

Issue 001 says "the `output_stream` class is completed and conforms to the
existing `write_trait` concept". **It cannot**: `write_trait<T>` requires
`obj.write(const char*, size_t) -> T&`, while the scaffold's `write` returns
`rw_result`. The scaffold is a *consumer* of a `write_trait` stream, not a
model of one. It is also asymmetric with the read side, which has no wrapper
class at all — `read_native_impl` is a pair of free-function overloads.

Decision: delete the scaffold; the write path is free functions
`write_native_impl(stream&, const T&, std::size_t)` overloaded on
`identified_as_constexpr_stream` vs `writeable`, exactly mirroring
`read_native_impl`. The library ships no concrete stream types in either
direction — consumers bring their own (`std::ofstream`, the tests'
`memstream`). The "constexpr counterpart" issue 001 asks for is therefore a
test-side writable `memstream`, not a header under `include/`.

**Issue 001 needs its wording amended** to "the `output_stream` scaffold is
removed and replaced by `write_native_impl` overloads that accept any
`write_trait` / `std_write_trait` / constexpr stream". Everything else in
001 stands.

### 1.2 Cycles and amalgamation

`scripts/amalgam.py` topologically sorts the include graph and emits each
file once; a cycle silently produces a wrong emission order, so the write
headers must form a DAG with the existing ones.

```
api/struct_write.hpp
  └─ cast/struct_write_impl.hpp
       ├─ field_list/field_list.hpp
       └─ field_write/field_writer.hpp
            ├─ field_write/write_impl.hpp
            │    ├─ stream/byte_order.hpp ── stream/stream_traits.hpp
            │    ├─ lib/memory/address_manip.hpp
            │    └─ error/cast_error.hpp
            ├─ field_write/derived_value.hpp
            │    ├─ field_list/field_list.hpp
            │    ├─ field_size/{field_size,field_size_deduce}.hpp
            │    └─ type_deduction/switch/switch.hpp
            ├─ field/field_traits.hpp
            └─ type_deduction/type/type_impl.hpp
```

The one back-edge that would close a cycle —
`field_writer.hpp` needing `struct_write_impl` to recurse into nested field
lists — is broken the same way the read side already breaks it:
`field_writer.hpp` **forward-declares** `template <typename F, typename
stream, auto endianness> struct struct_write_impl;` and never includes
`cast/struct_write_impl.hpp` (see `field_reader.hpp:161`). The declaration
suffices because the use is dependent.

No new external dependencies. No new std headers beyond those already pulled
in (`<expected>`, `<bit>`, `<utility>`, `<variant>`).

---

## 2. Pipeline shape

Four layers, one per header, mirroring read:

```
struct_write_le/be(stream&, const T&)                    -> cast_result   api/
  struct_write_impl<F, stream, endianness>{}(s, obj)     -> cast_result   cast/
    write_field<field_k, F>{value, obj}.write<E>(s)      -> rw_result     field_write/field_writer.hpp
      derive_value / verify / constraint_checker         -> rw_result     field_write/derived_value.hpp
      write_impl<E>(s, value, n)                         -> rw_result     field_write/write_impl.hpp
        write_native_impl(s, value, n)   [2 overloads]   -> rw_result
```

### 2.1 The fold

```cpp
template <auto metadata, typename... fields, typename stream, auto endianness>
struct struct_write_impl<struct_field_list_impl<metadata, fields...>, stream, endianness> {
  using S = struct_field_list_impl<metadata, fields...>;

  constexpr auto operator()(stream& s, const S& field_list) const -> cast_result {
    cast_result pipeline_seed{};
    return (
      pipeline_seed | ... | [&]() -> cast_result {
        const auto& value = static_cast<const fields&>(field_list).value;
        auto res = write_field<fields, S>(value, field_list).template write<endianness>(s);
        if(!res)
          return std::unexpected(cast_error{res.error(), std::string_view{fields::field_id.data()}});
        return {};
      }
    );
  }
};
```

Strict left-to-right order is guaranteed structurally, not by convention: a
binary left fold expands to `((seed | f0) | f1) | f2`, and each lambda is
invoked *inside* `operator|`, so `f0` completes before the enclosing call
that runs `f1` is entered. `operator|` short-circuits on the first failure.
This is the same mechanism `struct_cast_impl` relies on, which is what keeps
read and write in lockstep on ordering.

### 2.2 `write_field` binds by reference, not by field object

`read_field<T, F>` holds `T& field` (a whole `field<>` object) and uses
`field.value`. The write side instead holds the value:

```cpp
template <typename T, typename F>
struct write_field {                       // T = field<...>, F = struct_field_list_impl<...>
  const typename T::field_type& value;
  const F& field_list;
};
```

Forcing problem: the write source is `const`, and two field kinds have no
`field<>` object to point at — a variant alternative (`std::get<i>(variant)`)
and an optional's payload (`*opt`). Keeping read's shape would mean
materializing a `field<>` and **copying** the alternative, which for an
`as_vec` / `as_string` alternative is a heap allocation the spec forbids.
Binding the value by reference costs nothing and removes that whole class of
copies. `T` stays a template parameter, so the size spec, id and constraint
are all still reachable.

Specializations are keyed on exactly the read side's concepts —
`fixed_sized_field_like`, `variable_sized_field_like`,
`array_of_record_field_like`, `vector_of_record_field_like`,
`struct_field_like`, `optional_field_like`, `union_field_like` — so the two
directions cannot drift apart on which kinds exist. Partial-ordering between
them is already proven by `read_field`.

### 2.3 Constraint checking runs inside `write_field` (issue 003)

Read validates in the *fold*, after `read_field` returns
(`struct_cast_impl.hpp:44`). Write cannot: for a derived field the value
that matters is the derived one, which the fold never sees, and issue 003
requires rejection *before* the bytes are emitted. So the check moves into
`write_field::write`, immediately before the first byte of that field:

```cpp
if(!T::constraint_checker(value_to_write))
  return std::unexpected(error_reason::validation_failure);
```

Two consequences, both recorded deliberately:

- Write validates an **optional's payload** (via the recursive
  `write_field<field_base_type>`), which read does not — read only runs
  `maybe_field`'s own `no_constraint`. A stream that reads back fine can
  therefore fail to write. This is a read-side validation gap, not a write
  bug; fixing read is out of scope (the spec requires existing read tests to
  stay untouched) but it should be filed.
- Union `field_choices` are synthesized with `no_constraint`, so there is
  nothing to check on an alternative's own value — unchanged from read.

---

## 3. Endianness, allocation, and the constexpr/runtime split (delegated Q6)

The two paths diverge at exactly **one** function, the same place read
diverges:

```cpp
template <typename T, identified_as_constexpr_stream stream>
constexpr auto write_native_impl(stream& s, const T& obj, std::size_t n) -> rw_result;
template <typename T, writeable stream>
constexpr auto write_native_impl(stream& s, const T& obj, std::size_t n) -> rw_result;
```

Everything above it — `write_native`, `write_foreign_*`, `write_impl`, all
of `write_field`, the fold, and the public entry points — is a single
`constexpr` template body, generic over the stream type. Overload resolution
at the leaf selects the buffer path or the byte-pointer path. There is no
`if constexpr` on stream kind anywhere above the leaf and therefore **zero
duplication of field-kind dispatch**.

Issue 001's phrase "a single public entry point per byte order dispatches
internally to the constexpr-buffer path or the runtime-stream path" is
satisfied by this, and should not be read as asking for a branch in
`api/struct_write.hpp`.

`write_impl<endianness>` mirrors `read_impl`: `if constexpr` on
`deduce_byte_order<endianness>()`, then on `trivial<T>` vs `buffer_like<T>`.

**Foreign-endian buffers are written element-wise.** Read byteswaps in
place after reading; write cannot, because the source is `const`. The
alternatives were (a) stage a byteswapped copy of the whole buffer — a heap
allocation for `std::vector`/`std::string`, or (b) byteswap each element
into a stack scalar and write it. (b) is chosen: it allocates nothing, so
**the write path allocates nothing at all**, which is strictly stronger than
the spec's "no allocation beyond what byteswapping already requires".

Known pre-existing defect this touches: `read_foreign_buffer`
(`read_impl.hpp:69-77`) byteswaps `obj` instead of `elem` inside its loop
and cannot compile for a vector — foreign-endian buffer reads are broken
today. Issue 002 requires foreign-endian round-trip for arrays/strings, so
**issue 002 implicitly includes fixing `read_foreign_buffer`**; the write
mirror must implement the intended element-wise behaviour, not copy the bug.

Also pre-existing and mirrored rather than fixed: a field whose declared
width is narrower than its type (`basic_field` only requires `width <=
sizeof(T)`) is read from, and written to, the front `width` bytes of the
object, which is only correct when host order matches the declared order.
Read and write compose consistently on a given host; making it order-correct
is a separate change to both directions.

---

## 4. Derived vs. verified (delegated Q3, Q4, Q5; issues 004, 005, 007, 008, 009)

### 4.1 The obligation model

Every dependency in the schema is expressed as an **obligation**: some
field's *data* implies what some earlier field's *value* must be.

| Producer | Target | Value implied |
|---|---|---|
| `field<..., field_size<len_from_field<"n">>, ...>` (vec, string, vector-of-records) | `"n"` | the container's `.size()` (element count, matching read's `resize(len)` semantics) |
| `union_field<id, type<match_field<"t">, type_switch<cases...>>>` | `"t"` | `match_case_at<variant.index()>::value` |

An obligation is **unconditional** when its producer is a plain top-level
field, and **conditional** when the producer sits inside a `maybe_field` or
is a `union_field` alternative (it only exists when the optional is present
/ that alternative is held).

```
derived(id)  ⇔  id is the target of at least one *unconditional* obligation
```

Everything else is verified, never derived:

- `len_from_fields<callable, ids>` produces **no** obligation — an arbitrary
  N-ary callable has no inverse. Its source fields stay ordinary writable
  data.
- `parse_if` presence produces no obligation — presence is a predicate over
  siblings, not a stored slot.
- `type<compute<...>, switch>` and `type<ladder>` produce no obligation —
  the switch input is computed, and ladder branches are arbitrary
  predicates.

Why derivation must scan *forward*: the value of a derived field lives in
fields that come **after** it. `is_dependencies_resolved`
(`field_list_metadata.hpp:260`) already rejects any schema where a
dependency occurs after its dependent, so at the moment the writer reaches
field `k` every producer that obligates it is at index `> k` and is fully
populated in the (already complete) struct. A forward scan over the field
pack is therefore always well-defined — this is precisely what the spec's
"strict left-to-right write order" buys.

### 4.2 What the writer does, per field

| Situation | Action |
|---|---|
| plain data, no obligations | write stored value |
| derived target | **ignore stored value**; collect all *active* obligations, require agreement, check width, write derived value |
| target of conditional obligations only (not derived) | write stored value, but **verify** it against every currently-active obligation; mismatch → `validation_failure` |
| `len_from_fields` dependent | evaluate `compute_impl<compute<callable, size_t, ids>>` over the stored sources and **verify** against the container's real `.size()`; mismatch → `validation_failure` at the container field |
| shared-length fan-out (N ≥ 2 obligations on one target) | falls out of "require agreement" — no separate mechanism |
| `maybe<f, parse_if<...>>` | `compute_impl<field_presence_checker>{}(field_list)` vs `value.has_value()`; disagree → `validation_failure`; agree+present → recurse into `write_field<field_base_type>` bound to `*value`; agree+absent → emit nothing |
| `variance` with `type<match_field, switch>` | **no verification** — the discriminant was derived, so agreement holds by construction. Write the held alternative |
| `variance` with `type<compute, switch>` or `type<ladder>` | run the *existing* `deduce_type<guide>{}(field_list)`; propagate `type_deduction_failure` if no branch matches; `*idx != value.index()` → `validation_failure`; then write the held alternative |

Verification always reuses the read path's own machinery — `compute_impl`,
`deduce_type`, `evaluate_switch`, `evaluate_ladder`, `deduce_field_size` —
never a parallel reimplementation. All of them already take
`const struct_field_list_impl&`, so they work unchanged against the const
object being written.

Note the subtle case the obligation model handles for free: a field that is
both a `match_field` discriminant (unconditional ⇒ derived) *and* the
`len_from_field` source of a vector inside a `maybe` (conditional). It is
derived from the union, and the optional's length is verified against the
derived value when the optional is present. A naive "derived = union of
sources minus conditional sources" formulation would have dropped it into a
hole where it is neither derived nor verified.

**Issue 007 is slightly under-scoped**: it covers `len_from_fields` and
fan-out, but not conditional length sources (a `len_from_field` target whose
only dependent lives inside a `maybe` or a union alternative). Those are
neither derivable nor `len_from_fields`; they need the verify-when-active
path. Add that criterion to 007.

### 4.3 Derived-field detection and the `operator[]` constraint (issue 005)

Computed once, in `field_list_metadata.hpp`, so the writer and `operator[]`
can never disagree:

```cpp
static constexpr dep_vec unconditional_len_sources = ...;  // plain field<> with len_from_field
static constexpr dep_vec conditional_len_sources   = ...;  // maybe_field bases + union field_choices
static constexpr dep_vec switch_discriminants      = ...;  // union_field<id, type<match_field<t>, sw>>
static constexpr dep_vec derived_field_ids =
    switch_discriminants ∪ (unconditional_len_sources \ conditional_len_sources);
```

Each is a fold of per-field metafunctions in the same style as the existing
`extract_length_dependencies` / `extract_type_deduction_dependencies`, whose
partial specializations already distinguish exactly the shapes needed:
`field_size<len_from_field<src>>` vs `field_size<len_from_fields<c, ids>>`
(different specializations, so the non-invertible form is excluded for
free), and `type<match_field<t>, sw>` vs `type<compute<...>, sw>` vs
`type<ladder>`. Membership is tested with `find_index_if` from
`lib/algorithms/algorithms.hpp` — the same helper `static_map` uses.

```cpp
template <auto list_metadata>
constexpr auto is_derived_field(sv name) -> bool;

template <auto list_metadata, typename accessor>
concept field_is_derived_from_other_fields =
    is_derived_field<list_metadata>(as_sv(accessor::field_id));
```

`struct_field_list_impl` then carries three `operator[]` overloads:

| # | qualifier | extra constraint | returns |
|---|---|---|---|
| 1 | non-const | `!field_is_derived_from_other_fields<...>` | `auto&` (assignable) — existing overload, one term added |
| 2 | non-const | `field_is_derived_from_other_fields<...>` | `const auto&` |
| 3 | const | none (unchanged) | `const auto&` |

Overload 1 gains the `requires` term issue 005 mandates. Overload 2 exists
because without it the behaviour for a derived field on a *non-const* object
would be an accident of overload resolution (overload 3 is viable on a
non-const object via qualification conversion, so it would silently absorb
the call). Eight lines buy an explicit, self-documenting signature instead
of an emergent one.

Result: `s["len"]` reads fine on const *and* non-const objects;
`s["len"] = 5` fails to compile.

**Diagnostic quality — the honest position.** The failure surfaces as an
assign-to-const error (gcc: `assignment of read-only location`; clang:
`cannot assign to return value because function 'operator[]' returns a const
value`), with the source line echoing `"len"_f`. So the defect issue 005
names — "a wall of unsatisfied-constraint noise" — **does not occur at all**:
overload 2 is viable, so no candidate-rejection dump is ever printed. What
the message does *not* say by itself is *why* the field is read-only.

Two ways to close that last gap were evaluated:

- *Deleted overload for derived fields.* Rejected: deleted functions
  participate in overload resolution, so `s["len"]` would stop compiling for
  **reads** on a non-const object — and after `struct_cast` a user's struct
  is normally non-const. That breaks issue 005's own "reading a parsed
  length still works".
- *Proxy return type* (`read_only_derived_field<"len", T>`, implicitly
  convertible to `const T&`, `operator=` deleted). This does produce a
  message naming the field and saying read-only. Rejected: it changes
  `decltype(s["len"])` for derived fields on non-const objects, leaking a
  proxy into `auto` deduction, generic code and gtest matchers, and it moves
  enforcement off the `requires` term that issue 005 mandates onto a deleted
  operator — an abstraction whose only forcing problem is one line of
  message text. C++23 has no `= delete("reason")` on the gcc 13.1 baseline,
  so there is no cheap third option.

**Issue 005's last acceptance criterion should be narrowed** to: "assignment
to a derived field is rejected as an assignment to a const reference, with
no unsatisfied-constraint dump, and `UserGuide.md` documents the exact
message together with the reason and the fix." The concept name
`field_is_derived_from_other_fields` still carries the reason in any context
where the constraint is reported (e.g. `-fconcepts-diagnostics-depth`,
`requires`-expression probing).

Blast radius of the breaking change, measured: **zero in-repo**. No test
assigns through `operator[]`; the only reads of a `len`-shaped field
(`test/constexpr/constexpr_read.cpp:127`) go through a `constexpr` (hence
const) object, and that particular `"len"` is not even derived because its
`array_of_records` sibling is `size_dont_care`.

### 4.4 Deriving the value

```cpp
template <typename target, typename F> struct derive_value;   // -> expected<field_type, error_reason>
```

Specialized on `struct_field_list_impl<md, fields...>` so it can expand the
pack, it folds over the fields collecting active obligations for
`target::field_id`, then:

1. all obligations must agree — disagreement is the fan-out contradiction →
   `validation_failure`;
2. the value must fit the *declared width*, not just the field type —
   `width >= sizeof(size_t) || (v >> (width * 8)) == 0`, plus a
   `static_cast<size_t>(static_cast<field_type>(v)) == v` round-trip to
   cover signed/narrow field types. Failure → `validation_failure`. This is
   the "never silent truncation" requirement, and it applies identically to
   derived and verified lengths. (The round-trip test avoids pulling in
   `<limits>`.)

The discriminant obligation maps `variant.index()` to a `match_case` value
by folding over the cases with an `index_sequence`, exactly as
`read_variant_helper` maps an index to an alternative.

Only `fixed_sized_field_like` fields can be derived targets (a length or
discriminant is always a `basic_field`), so the derivation lives behind a
single `if constexpr (is_derived_target<T, F>)` inside that one
specialization of `write_field` — no extra partial specializations, hence no
two-parameter partial-ordering puzzles for MSVC to get wrong.

### 4.5 Does a `type_switch` discriminant still need its own field slot? (spec open question, issue 009)

**Yes — the slot stays, unconditionally.**

The discriminant occupies real bytes in the stream; if the write path
inferred it purely from the held alternative and emitted nothing, the output
would be missing those bytes and `struct_cast` could not read it back. The
schema is shared between directions, so the slot is also what the read path
resolves `match_field<"t">` against (`type_impl.hpp:34`:
`sfl[field_accessor<id>{}]`). Removing it would require a separate write-only
schema view — rejected in §8.

What changes is only *where the written value comes from*: derived from the
held alternative, not read from the slot. Round-trip symmetry is exact:

```
read :  bytes -> slot value -> evaluate_switch -> case index -> variant alternative
write:  variant alternative -> case index -> match_case value -> slot bytes
```

The inverse is total because `variant_from_type_conditions_v<cases...>`
builds `std::variant` in case order, so variant index `i` ↔ case `i`
positionally — no search, no ambiguity.

**But one prerequisite is missing.** The spec assumes "uniqueness already
enforced by `variance`'s existing `requires`". That `requires`
(`field_descriptors.hpp:75`, `has_unique_field_choices`) enforces uniqueness
of the case *types*, not the case *values*. Type uniqueness is what makes
`variant.index()` unambiguous; it does not stop two cases sharing the same
`match_case` value. If they did, `evaluate_switch` (first match wins) would
read back the *first* alternative after the writer emitted the second one's
value — a silent round-trip break.

Such a schema is already latently broken on read (the later alternative is
unreachable), so tightening is a strict improvement, not a breaking change.
Add to `variance` / `type_switch`:

```cpp
requires (has_unique_match_values<cases...>)   // pairwise == fold over the NTTP values
```

Fallback if the pairwise comparison proves awkward across all three
toolchains (the `auto` NTTPs are heterogeneously typed in principle):
document the requirement in `UserGuide.md` and drop the check. Add this to
issue 009.

For the non-invertible deducers there is no slot question at all —
`type<compute<...>, switch>` has no dedicated slot (its input is computed
from N ordinary fields), and `type<ladder>` has none either. Both are
verified with `deduce_type` per §4.2.

---

## 5. Error taxonomy (delegated Q2)

**Decision: no new `error_reason` enumerator.** The existing three partition
every write-side failure:

| Failure | Reason | Rationale |
|---|---|---|
| underlying stream write fails | `buffer_exhaustion` | spec-mandated; mirrors read's use for stream exhaustion |
| `constraint_checker` rejects the value being written | `validation_failure` | identical meaning to read |
| presence predicate disagrees with `has_value()` | `validation_failure` | cross-field data inconsistency |
| `len_from_fields` callable disagrees with real size | `validation_failure` | ditto |
| fan-out obligations contradict | `validation_failure` | ditto |
| ladder / computed-switch selects a different alternative than held | `validation_failure` | the deduction *succeeded*; the struct is inconsistent |
| no ladder branch / no `match_case` matches at all | `type_deduction_failure` | produced by the existing `deduce_type`, propagated unchanged |
| derived/verified length exceeds the declared width | `validation_failure` | a value the schema cannot represent |

Considered and rejected: `length_overflow` and `inconsistent_field_state`.
Both were rejected on the same ground — a finer taxonomy is only actionable
if the caller can tell *which two things* disagree, and `cast_error` carries
one `failed_at` field name, not two. Adding the second name is a change to
`cast_error` that affects the read path too and is out of this spec's scope.
Meanwhile the distinction callers actually act on — "the sink died" vs "my
data is wrong" vs "my schema can't classify this" — is already exactly the
existing three. Introducing an enumerator now would also churn any consumer
switching exhaustively over `error_reason`.

What would change this decision: `cast_error` gaining a second field name,
at which point splitting `validation_failure` into
own-value-vs-cross-field becomes genuinely informative.

### 5.1 Amendment 2026-08-07 — decision reversed in part

Owner's call during issue 007: a fourth enumerator,
**`found_contradicting_length`**, is added. The two rows below move to it:

| Failure | Reason |
|---|---|
| `len_from_fields` callable disagrees with real size | `found_contradicting_length` |
| fan-out obligations contradict | `found_contradicting_length` |

Everything else in the table above stands. In particular
`derived/verified length exceeds the declared width` stays
`validation_failure` — the schema cannot represent the value, which is not
the same as two fields disagreeing about it.

The argument the original decision rested on is not withdrawn: a caller
still cannot tell *which* dependent disagreed, because `cast_error` still
carries one `failed_at`. What changed is the judgement that
"a contradicting length" is worth distinguishing from "a value failed its
own constraint" even without the second name, since the two call for
completely different fixes. The name is deliberately narrow rather than
`inconsistent_field_state`: it says what actually happened, and it does not
pre-commit the presence-predicate and discriminant mismatches of issues 008
and 009 to this enumerator. Those stay `validation_failure` until there is
a reason to split them.

Richer error types — a second field name, or a payload carrying both sides
of the disagreement — remain future work and are out of this spec's scope.
The enumerator is appended, not inserted, so existing values are unchanged.

---

## 6. Mandated decision categories

### State and data lifecycle

The only observable state is bytes already pushed to the caller's stream, so
partial completion **is** visible and the spec forbids rollback. The unit of
atomicity is therefore **one field, not the write**:

- every derivation, verification and constraint check for field `k` runs
  *before* the first byte of field `k` is emitted (this is why constraint
  checking moved out of the fold, §2.3);
- consequently a failure attributed to field `k` leaves exactly fields
  `0..k-1` in the stream and nothing of `k` — a well-defined prefix, which
  is the strongest guarantee available without buffering.

The only exception is a stream failure mid-field (a short write inside a
buffer): the underlying stream decides how much it took, and the library
reports `buffer_exhaustion`. Not reversible, not replayable by the library,
not audited beyond `cast_error{reason, failed_at}`. No files, no persistent
stores, no external calls, no in-memory state survives the call — the
`const T&` input is never mutated and no writer object outlives its
`operator()`.

### Error propagation

Three seams, matching read exactly:

1. **Stream primitives → `rw_result`.** `write_native_impl` maps a falsy
   stream to `error_reason::buffer_exhaustion`. This layer knows nothing
   about fields and never names one.
2. **Field layer → `rw_result`.** `write_field`, `derive_value` and the
   verification helpers speak `error_reason` only. Recursion into a nested
   field list is the one place representation changes *downward*:
   `struct_write_impl` returns `cast_result`, and
   `write_field<struct_field_like>` **discards `failed_at`** and returns
   `std::unexpected(err.failure_reason)` — precisely what
   `read_field<struct_field_like>` does (`field_reader.hpp:177-179`).
3. **List layer → `cast_result`.** The fold in `struct_write_impl` attaches
   `fields::field_id` to whatever `rw_result` came back. This is the only
   place a field name is ever attached, in either direction.

Consequence worth stating: for a nested violation, `failed_at` names the
**outermost** field of the failing nesting level, not the innermost field.
**Issue 006's criterion "`failed_at` identifying the nested field" is not
deliverable** without giving `cast_error` a path rather than a name, which
would change the read path's error shape too. Narrow issue 006 to "names the
outer record field, matching read-side behaviour", or file the `cast_error`
change as separate work.

No exceptions are thrown or caught anywhere. `std::get<i>` is only ever
called when `index() == i`, so `bad_variant_access` is unreachable.

### Concurrency and ownership

**Single-threaded; no shared mutable state; no synchronization.** Stated
rather than assumed, because this is a header-only library that will be
reused in contexts its author didn't pick:

- the only mutable state touched is the caller's stream, *borrowed* by
  reference for the duration of the call and owned by the caller;
- `const T& obj` is never mutated — the write path takes const references
  all the way down, which is also what makes the read path's `operator[]`
  machinery reusable verbatim;
- every writer object (`write_field`, `derive_value`,
  `write_variant_helper`) is a stack-local non-owning aggregate of two
  references, destroyed before its caller returns;
- no globals, no statics with mutable state, no thread-locals. The only
  namespace-scope state is `constexpr` metadata and `meta::type_id`'s
  friend-injection counters, both compile-time only.

Therefore: two concurrent `struct_write_*` calls on **distinct** streams are
independent and safe with no locking; two calls sharing one stream are the
caller's problem, exactly as with `std::ofstream` itself. The library adds no
serialization requirement of its own.

### Reuse

Used rather than reimplemented: `deduce_field_size` (both the constexpr and
field-list forms), `compute_impl`, `deduce_type` / `evaluate_switch` /
`evaluate_ladder`, every `field_traits` concept, `field_list_metadata`'s
tables and extraction-metafunction style, `cast_error` / `rw_result` /
`cast_result`, `static_vector` / `static_optional` / `fixed_string` /
`find_index_if` from `include/lib/`, `as_byte_buffer`, `std::byteswap`,
`meta::type_of`. Crucially, **all validation reuses the read path's
evaluators** — a parallel presence/deduction implementation would be exactly
the drift that lets write accept what read rejects.

Introduced here and worth factoring for reuse:

- `stream/byte_order.hpp` — direction-neutral, both paths now share it;
- `operator|` on `cast_result` / `rw_result` moved to `error/cast_error.hpp`
  — the pipeline idiom is now available to any future driver;
- `is_derived_field<metadata>(sv)` in the metadata layer — single source of
  truth for both `operator[]` and the writer;
- the obligation fold — the place any future invertible dependency plugs in.

Extending the established pattern, not adding a parallel one: `field_write/`
mirrors `field_read/` file-for-file, `write_field` mirrors `read_field`
specialization-for-specialization, `struct_write_impl` mirrors
`struct_cast_impl` fold-for-fold, `api/struct_write.hpp` mirrors
`api/struct_cast.hpp`. Forward-declaration to break the driver↔dispatch
cycle is the read side's existing technique, reused verbatim.

### Extension points

Deliberately provided, with the contract that keeps them stable:

1. **`write_field<T, F>` partial specialization** — the field-kind seam,
   open exactly like `read_field`. Contract: `constexpr auto write<auto
   endianness, typename stream>(stream&) const -> rw_result`; must run the
   field's `constraint_checker` before emitting bytes; must emit nothing if
   it will fail; must recurse (not flatten) for nested field lists. A new
   field kind adds one `read_field` and one `write_field` specialization and
   touches nothing else.
2. **`derivation_obligation<producer, target_id>`** — the seam for new
   *invertible* dependencies. Contract: return `static_optional<size_t>`,
   empty when inactive; never observe write order.
3. **`output_stream_like`** — consumers plug in any stream; the library
   ships none.

Deliberately **not** extension points: there is no hook for "how to derive a
value" (derivation is schema-implied, and a user hook would reintroduce the
desynchronization the feature exists to eliminate), no per-type serializer
customization, and no policy object for endianness (the schema declares it).

### Build vs. buy

- **Serialization framework** (zpp::bits, cista, Glaze, Boost.PFR-based
  approaches): evaluated, rejected. The schema is s2s's own descriptor DSL,
  not a C++ aggregate, so reflection-based libraries have nothing to reflect
  on; none can express derived lengths or discriminant inversion; and each
  is an external dependency the spec forbids for a header-only,
  amalgamated-to-one-file library.
- **Byte swapping**: bought — `std::byteswap` (C++23), already used by read.
- **Error plumbing**: bought — `std::expected`, already the project idiom.
- **Constexpr byte reinterpretation**: bought — `std::bit_cast`, already
  used by `as_byte_buffer`.
- **Compile-time containers/strings**: already built in `include/lib/`, per
  the project override on template-facing vocabulary types; not rebuilt.
- **Everything else** (obligation scan, field-kind dispatch, width checks):
  built, because it is schema-specific by construction — there is nothing
  external that understands `field_size<len_from_field<"n">>`.

### Abstractions introduced

Each with the specific problem forcing it:

| Abstraction | Forcing problem |
|---|---|
| `write_field<T, F>` | field-kind dispatch must be open (new kinds) and must stay 1:1 with `read_field` so the directions cannot diverge |
| `derivation_obligation` / `derive_value` | derived values live in fields that come *later*; sequential writing cannot produce them, so a forward scan is unavoidable, and fan-out agreement needs the obligations as a set rather than a single lookup |
| `is_derived_field` in the metadata layer | `operator[]`'s constraint and the writer must give the same answer; two independent scans would drift |
| `field_is_derived_from_other_fields` concept | the project override mandates concepts over ad-hoc constraints, and the name is what carries the reason into diagnostics |
| `stream/byte_order.hpp` | not an abstraction — a relocation of existing code to a direction-neutral home |

Rejected as unnecessary (no forcing problem):

- the `output_stream` wrapper class — read has no counterpart and needs none
  (§1.1);
- an endianness policy/traits class — `if constexpr` in `write_impl` mirrors
  read and is sufficient;
- a `writer` base class or CRTP for the `write_field` specializations — they
  share no code, only a shape;
- a proxy reference type for derived fields (§4.3).

### Alternatives rejected

| Alternative | Why rejected |
|---|---|
| Two-pass write (size pass, then emit) | would permit rollback and non-invertible derivation, but the spec forbids a dry-run/size-query API and mandates write-once/fail-fast |
| Buffer-then-flush for atomicity | needs an allocation proportional to the struct; spec forbids allocation and forbids rollback |
| Trust the stored length and only validate it | spec mandates *unconditional* derivation for invertible cases — trusting is exactly the desynchronization bug the feature removes |
| Invert the `len_from_fields` callable | impossible in general for an arbitrary N-ary user callable |
| Drop the `type_switch` discriminant's field slot | the bytes must still be emitted for the stream to read back; the read path resolves `match_field` against that slot (§4.5) |
| A separate write-only schema type | doubles the schema surface and breaks the "one schema, both directions" premise that makes round-trip fidelity checkable |
| `read_field`'s shape for `write_field` (hold a `field<>` object) | forces copies of variant alternatives and optional payloads — heap allocation the spec forbids (§2.2) |
| Deleted `operator[]` overload for derived fields | breaks *reads* of derived fields on non-const objects (§4.3) |
| Proxy return type for derived fields | leaks a proxy into `decltype`/`auto`/generic code for one line of message quality (§4.3) |
| New `error_reason` enumerators | not actionable without a second field name in `cast_error` (§5) |
| Constraint checking in the fold, as read does it | the fold cannot see a derived value, and issue 003 requires rejection before bytes are emitted (§2.3) |
| Keeping `cast_endianness` in `read_impl.hpp` and including it from the write path | a `field_write → field_read` edge — wrong direction, and drags the whole read implementation into every write TU |

---

## 7. Test surface

Mirrors `test/runtime/` + `test/constexpr/`, one file per field kind,
registered in the respective `CMakeLists.txt`; existing read tests untouched.

- Round-trip per kind, both byte orders: populate → `struct_write_le/be` →
  `struct_cast_le/be` → compare.
- Failure cases (runtime): constraint violation; length-width overflow;
  `parse_if` presence mismatch in **both** directions (predicate true +
  empty, predicate false + engaged); `len_from_fields` disagreement; fan-out
  contradiction; ladder/held-alternative mismatch.
- Compile-fail: assignment to a `len_from_field` target and to a
  `type_switch` discriminant; const read of both still compiles. Needs a
  CMake compile-fail harness (`try_compile` / `PASS_REGULAR_EXPRESSION`) —
  the repo has none today, so **issue 005 must include standing it up**.
- Constexpr: a writable `memstream` (write into `std::array`) plus
  `static_assert`-level round-trip, matching `constexpr_read.cpp`.

---

## 8. Effect on the approved slice ordering

The dependency order 001 → 002/003/004 → 005/006/007/008 → 009 → 010 → 011
is implementable as approved. Six scope corrections, none of which reorder
anything:

1. **001** grows: the `output_stream` criterion must be restated as removal
   (§1.1); it also carries the `output_stream_like` fix, the `operator|`
   relocation, the `as_byte_buffer` const change, and the
   `stream/byte_order.hpp` extraction. All are prerequisites for the first
   byte to be written and belong in the walking skeleton.
2. **002** implicitly includes fixing `read_foreign_buffer`, without which
   its "round-trip verified for both byte orders" criterion cannot pass
   (§3).
3. **003**: constraint checking lands in `write_field`, not the fold, and
   write validates optional payloads that read does not — document the
   asymmetry (§2.3).
4. **004** introduces `derived_field_ids` in the metadata and the obligation
   fold; **005** then only wires them into `operator[]`. The 004 → 005 order
   is right, but the metadata work sits in 004, not 005.
5. **005**: narrow the diagnostic criterion (§4.3) and add the compile-fail
   harness to its scope (§7).
6. **006**: `failed_at` names the outer record field, not the nested one —
   narrow the criterion or file the `cast_error` change separately
   (§ Error propagation).
7. **007**: add conditional (maybe-/union-scoped) length sources to scope
   (§4.2).
8. **009**: add the `match_case`-value uniqueness requirement (§4.5), which
   the spec incorrectly assumed already existed.
