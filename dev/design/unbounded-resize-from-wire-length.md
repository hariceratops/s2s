# Design: bounding variable-sized field allocation (`max_bytes`)

Spec: `dev/inbox/unbounded-resize-from-wire-length.md` (unfrozen for the
default-ceiling revision; re-freezing is the user's move).
Issues served, in order: `dev/issues/046-checked-multiply-for-the-byte-count.md`,
then `dev/issues/047-bound-container-allocation-with-max-bytes.md`.
This design serves those two slice boundaries as given; it proposes no
re-slicing.

Sits directly on top of `dev/design/schema-api-verbosity.md` (043–045), which
landed on the branch beneath this one. Everything below obeys that design's
governing rule verbatim: **a class-type NTTP is a non-deduced context in a
partial specialization, so anything a trait pattern-matches on must live in the
value's *type*, never in its value.**

---

## 0. The shape in one page

| Piece | Where | What |
|---|---|---|
| `max_bytes<N>` | `field_size/field_size.hpp` | `inline constexpr auto max_bytes = max_byte_count<N>{}` — value of an empty type carrying `N` in the type |
| `use_default_bound` | `field_size/field_size.hpp` | `use_default_bound_t{}`, the default fifth argument of `field`; resolves to `default_max_bytes` |
| `bound_option_like` / `boundable_field_option_like` | `api/field_descriptors.hpp` | third option kind; only three descriptors accept it |
| `bound_in_pack` / `bound_of_pack` | `api/field_descriptors.hpp` | same scan shape as `size_in_pack` / `constraint_in_pack` |
| fifth parameter `auto max_byte_bound` | `field/field.hpp` | defaulted to `use_default_bound`; how the bound reaches the reader |
| `checked_byte_count<element, ceiling>(len)` | `field_read/read_impl.hpp` | the single arithmetic + bound gate, shared by both resize sites |
| `error_reason::excessive_length` | `error/cast_error.hpp` | appended enumerator; `cast_error` unchanged |

Slice split:

- **046** lands `checked_byte_count`, the `excessive_length` enumerator, and its
  use inside `read_native`'s `variable_sized_buffer_like` overload with the
  ceiling defaulted to `SIZE_MAX`. No schema surface, no fifth parameter, no
  `field_descriptors.hpp` change. It is a self-contained correctness fix and
  compiles/tests on its own.

  The ceiling stays `SIZE_MAX` here deliberately, even though the default is
  about to become 16 MiB: 046 claims to fix an overflow, not to bound anything,
  and defaulting it low would quietly make that claim false-but-passing. 047 is
  where "bounded" becomes true.

  A consequence to carry into 046's tests: once `default_max_bytes` lands, a
  wrapping length is rejected by the *bound* long before it can overflow, so the
  pure-overflow path is only reachable with the ceiling raised to `SIZE_MAX`.
  046's overflow case must therefore pin that ceiling explicitly rather than
  relying on the default, or it becomes a test of nothing the moment 047 lands.
- **047** lands the option vocabulary, the pack classifier, the fifth parameter
  and its threading, and the `vector_of_records` call site — supplying a real
  ceiling to the helper 046 already installed.

The seam between them is deliberately the *ceiling argument*: 046 builds the
gate with the widest possible ceiling, 047 narrows it per field. Nothing 046
writes is rewritten by 047.

---

## 1. `checked_byte_count` — one gate, both slices, both sites

```cpp
// read_impl.hpp
// Safety is on by default: a field that declares nothing still gets a ceiling.
// The macro is the one global knob — raise it, or set it to SIZE_MAX to turn
// the defaults off entirely. It cannot disable an explicitly declared
// max_bytes, which is the schema author's intent rather than the library's
// guess.
#ifndef S2S_DEFAULT_MAX_BYTES
#define S2S_DEFAULT_MAX_BYTES (16u * 1024u * 1024u)
#endif

inline constexpr std::size_t default_max_bytes = S2S_DEFAULT_MAX_BYTES;

template <typename element, std::size_t ceiling = default_max_bytes>
constexpr auto checked_byte_count(std::size_t len) -> std::expected<std::size_t, error_reason> {
  if(len > ceiling / sizeof(element))
    return std::unexpected(error_reason::excessive_length);
  return len * sizeof(element);
}
```

Three properties this shape is chosen for, none of them incidental:

**The check is a division, not a multiplication.** `len * sizeof(element)` is
never evaluated on an unvalidated `len` — not even to test it. `ceiling /
sizeof(element)` is a compile-time constant (both operands are), so the runtime
cost is one comparison. This is what makes the overflow test and the bound test
*the same test*: with `ceiling == SIZE_MAX` the comparison fires exactly when
the product would wrap; with `ceiling == N` it fires when the product would wrap
**or** exceed `N`. 047 therefore adds no second check anywhere — it changes one
template argument.

**`sizeof(element)` is never zero**, so no guard is written for it. C++ has no
zero-sized complete object type; a defensive `if` there would be error handling
for an impossible case.

**Inclusivity falls out.** `len > ceiling / sizeof(element)` accepts a byte
count exactly equal to `N` when `N` is a multiple of `sizeof(element)`, which
is what the spec's "N accepted, N+1 rejected" means. Where `N` is *not* a
multiple — `max_bytes<10>` on a `vec<u32>` — the largest accepted count is 2
(8 bytes), and 3 (12 bytes) is rejected. That is the correct reading of an
inclusive byte ceiling and needs a test case, because integer division is where
an off-by-one would hide.

**Why one helper and not two per-site checks** (the spec left this open): the
two sites disagree about what they do with the *product* — `read_native` needs
it as the read length, `vector_of_records` has no use for it (a record's wire
size is not `sizeof(record)`) — but they agree exactly on the *predicate*. A
shared predicate with a discarded return at one site keeps the single place
where "is this length admissible" is decided; two checks would let the two sites
drift on the division-vs-multiplication point, which is the whole soundness
argument. The records site discards the product deliberately, and that deserves
a `// WHY` comment at the call, not silence.

### Site 1 — `read_native`, `variable_sized_buffer_like` overload

```cpp
template <std::size_t ceiling = default_max_bytes, variable_sized_buffer_like T, input_stream_like stream>
constexpr auto read_native(stream& s, T& obj, std::size_t len_to_read) -> rw_result {
  auto byte_count = checked_byte_count<typename T::value_type, ceiling>(len_to_read);
  if(!byte_count)
    return std::unexpected(byte_count.error());
  obj.resize(len_to_read);
  if constexpr(identified_as_constexpr_stream<stream>) { /* unchanged */ }
  else return read_native_impl(s, obj, *byte_count);
}
```

The check goes **above the `resize`, above the constexpr/runtime branch**. Both
consequences matter: no allocation proportional to an unvalidated length happens
in either mode, and the constexpr path — which today computes no product at all
and so has no overflow bug — is nevertheless where the bound is enforced too.
That is what makes the feature testable under `add_ut_test`'s compile-time mode
at all (design question 4), and it is why the check cannot live in the runtime
branch beside the product it fixes.

`sizeof(T{}[0])` is replaced by `typename T::value_type`. `T{}` materialises a
container in an unevaluated operand to get at an element type both `std::vector`
and `std::string` publish directly; the replacement is not cosmetic, it removes
a default-construction expression from a constexpr-critical path.

**The check lives at the allocation site, not at the caller.** A ceiling passed
down and checked here makes `read_native` locally sound for every caller,
including `read_foreign_buffer`. Checking in `read_field` instead would leave a
wrapping multiply inside `read_native` justified only by a caller-side
invariant — which is precisely the class of reasoning 046 exists to delete.

### Site 2 — `vector_of_records`, `field_reader.hpp:153`

```cpp
auto len_to_read = deduce_field_size<field_size>{}(field_list);
// Validate the footprint before reserving it; the product is not the wire size
// here, because records may carry variable-length subfields.
auto footprint = checked_byte_count<vector_elem_type, bound_of<T>>(len_to_read);
if(!footprint)
  return std::unexpected(footprint.error());
field.value.resize(len_to_read);
```

`sizeof(vector_elem_type)` is the in-memory footprint of one record, per the
spec's explicit decision that memory — not wire size — is the denominator here.
No special case for `vector_of_records` results.

### Threading the ceiling through `read_impl`

`read_impl` gains a defaulted NTTP after `endianness`, so its two existing call
sites (`field_reader.hpp:33` and `:50`) keep compiling verbatim:

```cpp
template <std::endian endianness, std::size_t ceiling = default_max_bytes, typename T, input_stream_like stream>
constexpr auto read_impl(stream& s, T& obj, std::size_t N) -> rw_result;
```

In the host branch it dispatches with `if constexpr(variable_sized_buffer_like<T>)`
so that only the resizing overload of `read_native` is passed a ceiling; the
`constant_sized_like` overload does not gain a parameter it would ignore.
`read_foreign_buffer` (already constrained to `buffer_like`) forwards the
ceiling to `read_native`. Rejected alternative: giving both `read_native`
overloads the parameter so `read_impl` needs no `if constexpr`. That trades one
`if constexpr` for a permanently ignored parameter on the fixed-size path, and
an ignored parameter is the thing a later reader mistakes for a bound that is
being enforced.

---

## 2. How the bound reaches the read path (design question 1)

**Decision: a fifth NTTP on `field`, `auto max_byte_bound = use_default_bound`.**

```cpp
template <fixed_string id, typename T, auto size, auto constraint_on_value,
          auto max_byte_bound = use_default_bound>
struct field {
  ...
  static constexpr auto field_max_bytes = max_byte_bound;
};
```

The bound must be part of the field's *type*: two `vec_field`s differing only in
their ceiling must be different types, or a trait keyed on the type could not
recover the ceiling at all. That eliminates every "attach it from outside"
scheme (a trait specialized per descriptor, a registry, a runtime parameter on
the public API) before the alternatives are even compared. What remains is a
choice of *which* type carries it.

### The blast radius, measured

Adding the parameter breaks nothing that *constructs* a `field` — the default
covers `to_field_choice`, `create_field_from_array_of_records`,
`create_field_from_vector_of_records`, `magic_*`, `struct_field`, and every
schema in `test/` and `examples/`. It breaks only **partial specializations that
pattern-match `field<id, T, size, constraint>`**, because a template-id with a
defaulted argument matches only fields carrying the default. Exhaustive list —
17 sites, each a one-parameter mechanical edit:

| File | Sites |
|---|---|
| `field/field.hpp` | `to_optional_field` (:31), `no_variance_field` (:44) |
| `field/field_traits.hpp` | `is_fixed_sized_field` (:17), `is_array_of_record_field` (:36), `is_variable_sized_field` (:58), `is_vector_of_record_field` (:78), `is_struct_field` (:98), `is_optional_field` (:126) |
| `field_list/field_list_metadata.hpp` | :44, :53, :62, :72, :81, :140, :237 |
| `field_write/derived_value.hpp` | `len_obligation` (:27), :92 |

All but one ignore the new parameter. The exception is `to_optional_field`,
which must **forward** it, so `maybe<vec_field<..., max_bytes<N>>>` keeps its
bound on the optional field type as well as on `field_base_type` (the reader
reaches the bound through `field_base_type`, so forwarding is about the two
types not disagreeing rather than about the read working).

This is the same blast radius, in the same four files, that 044 already paid
and that `dev/design/schema-api-verbosity.md` §2 enumerated. It is mechanical,
uniform, and fully covered by the existing suite: any missed site drops a field
out of its classification trait and fails loudly at the first schema that uses
it, rather than silently.

### Alternatives rejected

**Wrap the size: `bounded_size_t<len_from_field<"n">, N>`.** Fewer edit sites
(~10) and no change to `field`'s arity. Rejected on meaning, not count. It puts
an allocation ceiling — a *read-path* concern the spec explicitly scopes away
from the write path — inside the size vocabulary that `len_obligation`,
`derived_value`, `field_list_metadata` and `field_writer` all consume, forcing
every one of `is_variable_size`, `is_computed_size`, `len_source_of`,
`deduce_field_size` (both constrained specializations) and
`size_type_of<size>::f` to learn to unwrap. Each unwrap is a place a future size
kind can be added and silently not unwrapped, which fails open. A ceiling is not
a size; making it one to save seven mechanical edits trades a permanent
conceptual muddle for a one-commit convenience.

**Wrap the field: `bounded_field<field<...>, N>` deriving from `field`,
following `maybe_field`.** This is an established pattern here, but it creates a
new field *kind* that every `is_*_field` trait and all seven
`field_list_metadata` patterns must see through — a blast radius no smaller than
the fifth parameter, in exchange for a second wrapper interacting combinatorially
with `maybe_field` (`maybe<bounded<vec_field<...>>>` vs
`bounded<maybe<...>>`). The fifth parameter has no ordering to get wrong.

**A runtime ceiling parameter threaded from `struct_cast_le`.** Rejected by the
spec (no library-wide default, no template parameter on `struct_cast_*`), and it
would make the ceiling invisible in the schema, which is where the spec wants it
readable.

---

## 3. The option vocabulary and its classifier (design question 3)

### The value and its type — `field_size.hpp`

```cpp
struct use_default_bound_t {};
template <std::size_t N> struct max_byte_count { static constexpr std::size_t value = N; };

inline constexpr auto use_default_bound = use_default_bound_t{};
template <std::size_t N> inline constexpr auto max_bytes = max_byte_count<N>{};

template <typename B> struct is_allocation_bound            { static constexpr bool res = false; };
template <std::size_t N> struct is_allocation_bound<max_byte_count<N>> { static constexpr bool res = true; };
template <typename B> inline constexpr bool is_allocation_bound_v = is_allocation_bound<B>::res;
template <typename B> concept allocation_bound_like = is_allocation_bound_v<B>;

template <typename B> struct bound_in_bytes                 { static constexpr std::size_t value = default_max_bytes; };
template <std::size_t N> struct bound_in_bytes<max_byte_count<N>> { static constexpr std::size_t value = N; };
```

`N` lives in the **type**, not in the value, following 045's rule rather than
`byte_count`'s exception. `byte_count` earns its exception because no trait
matches on a width and the value form buys the `_B` literal; here a trait
(`bound_in_bytes`) does match on `N` in order to hand it to `read_impl` as a
template argument, so the rule applies in its plain form. The spelling the spec
fixes — `s2s::max_bytes<4096>` — is angle-bracketed already, so nothing is lost.

`use_default_bound_t` rather than `max_byte_count<SIZE_MAX>` as the default: "no bound
declared" and "a bound that happens to be huge" are different statements, and
only the first should be able to say so in a diagnostic. `bound_in_bytes`
collapses them to the same ceiling at exactly one point, which is the only place
the collapse is safe.

This lives in `field_size.hpp` (not `field_descriptors.hpp`) because `field.hpp`
needs `use_default_bound` for its default argument and already includes `field_size.hpp`;
`field_descriptors.hpp` is downstream of both. Placement is a header-dependency
consequence, not a taxonomy claim — a bound is not a size, and the file's
comment should say so where the new block starts.

### The classifier — `field_descriptors.hpp`

Third option kind, in exactly the machinery 045 built:

```cpp
template <typename B, typename T>
concept bound_option_like = allocation_bound_like<B>;

template <typename O, typename T>          // unchanged
concept field_option_like = size_option_like<O, T> || constraint_option_like<O, T>;

template <typename O, typename T>
concept boundable_field_option_like = field_option_like<O, T> || bound_option_like<O, T>;
```

`bound_in_pack` mirrors `size_in_pack` line for line (scan, first match, default
`use_default_bound`), `bound_option_count` mirrors `size_option_count`, `pack_options`
gains `static_assert(bound_option_count<T, opts...> <= 1, "a field takes at most
one allocation bound")`, and `resolved_options` gains `bound` alongside `size`
and `constraint`. No new mechanism, no new shape.

`bound_option_like` takes `T` and ignores it, purely so it composes into the
disjunction with the other two and can appear in a placeholder constraint. That
unused parameter is a genuine wart; the alternative (a one-parameter concept)
cannot be written as `boundable_field_option_like<T> auto... opts`, which is the
property §"the diagnostic" below depends on.

### Rejecting `max_bytes` on a fixed-size field

**Only `vec_field`, `str_field` and `vector_of_records` take
`boundable_field_option_like<T> auto... opts`. Every other descriptor keeps
`field_option_like` or `constraint_option_like` unchanged.**

`max_bytes<N>` on `basic_field`, `fixed_array_field`, `c_arr_field`,
`fixed_string_field`, `c_str_field`, `array_of_records` or `struct_field` then
fails the **per-element placeholder constraint** — the identical mechanism, and
the identical "no operand of the disjunction is satisfied" wording, that already
rejects an unrecognised entry. Not a `static_assert`, not a bolted-on check; the
spec's requirement is met by the classifier declining to classify.

The spec frames the rejection as "a relationship between two pack entries" (a
bound is meaningful only beside a `variable_size_like` size), and observes that a
per-element concept cannot express a relationship. The resolution is that **the
relationship is already decided by the descriptor**, one level up:

- The three container descriptors each carry `requires
  variable_size_like<size_type_of<size_of_pack<...>>>` today. Within them a
  fixed size is *already* unspellable, so no relationship remains to check —
  accepting a bound there is unconditionally correct.
- Every other descriptor's size is fixed by the descriptor itself (from `T`, from
  `N`, or `size_dont_care`), so a bound there is unconditionally wrong.

So the relationship is not lost, it is *hoisted* to the only place where it is
constant, and the per-element concept is then sufficient. A schema author who
writes `vec_field<"v", u8, max_bytes<16>>` with no size option is rejected by the
existing `variable_size_like` requires-clause — a bound with nothing variable to
bound still cannot compile, by a different sentence.

Rejected alternative: keep one `field_option_like` for all descriptors and add
`requires` logic relating the resolved bound to the resolved size. It compiles,
but the failure surfaces as an unsatisfied requires-clause on the alias — the
fold-style diagnostic 045 explicitly measured and rejected — instead of naming
the offending entry.

### The diagnostic property, preserved

Per-element, never a fold. `boundable_field_option_like<T> auto... opts`
substitutes `decltype(opt)` first, which is why every one of these concepts takes
the option's *type* as its first parameter and why a value-first concept could
not be used here at all. Adding a third disjunct widens the "both things it could
have been" line to three; that is the intended, and the only, change to the
existing wording.

---

## 4. Constexpr preservation (design question 4)

Nothing introduced is non-constant-evaluable, and the pieces that could have been
were chosen out:

- `checked_byte_count` is a comparison, a division of two constants and a
  multiply, returning `std::expected<std::size_t, error_reason>` — a type this
  codebase already returns from constant evaluation on every read.
- `std::numeric_limits<std::size_t>::max()` is `constexpr`; `<limits>` is a new
  include in `read_impl.hpp`.
- Placing the check above the constexpr/runtime branch in `read_native` is what
  makes the reject path reachable under `UT_COMPILE_TIME_ONLY` at all. Had it
  gone in the `else` beside the product, the compile-time tier could only ever
  test the accept path, and `test/ct_coverage_check.cmake` would pass while
  covering nothing.
- `max_byte_count<N>` is an empty class with only static members and no bases:
  literal and structural, hence a valid NTTP — the same argument
  `dev/design/schema-api-verbosity.md` §1 verified for `size_from_fields_t` and
  relies on for `no_constraint<T>{}`.
- Test lambdas must not capture (bound values must be `constexpr` locals or
  literals inside the lambda, not captured from the enclosing scope), or ut skips
  them at compile time with no diagnostic and only `*_coverage` notices.

---

## 5. Decisions, stated explicitly

### Safety is on by default (revises the frozen spec)

The spec this design was first written against made the ceiling opt-in and
recorded "safe is off by default" as deliberate. That has been reversed, and
the reversal is what `default_max_bytes` implements.

The reasoning is the failure mode, not the number. "Too low a default" fails
*closed*: a legitimate oversized field is rejected loudly, `failed_at` names it,
and the fix is one `max_bytes` declaration in the schema. "No default" fails
*open*, silently, in precisely the case nobody thought about. A wrong default
that announces itself beats no default that does not, so the "we would have to
pick a number" objection is not the blocker it first looked like.

It costs nothing at run time. The ceiling is a compile-time constant and the
gate is already one comparison against it, so a default is a different constant
in the same comparison, not an extra branch.

16 MiB is a judgement call, recorded so it can be argued with rather than
inherited: far above what fields in real binary formats carry, far below what a
corrupt `u32` can claim, and survivable as an accidental allocation on any
hosted target.

**`S2S_DEFAULT_MAX_BYTES` is the only global knob, and it cannot reach an
explicitly declared `max_bytes`.** Setting it to `SIZE_MAX` turns the defaults
off wholesale, which is the "profile" escape hatch without a second mechanism to
build or test. A declared bound stays in force regardless: a default is the
library guessing, a declaration is the author's intent, and a build flag that
silently discards a safety limit someone wrote by hand is the kind of thing that
turns up in a postmortem. No switch disables declared bounds.

A separate on/off profile was considered and rejected. The gate is one
predictable, almost-always-not-taken comparison sitting above a heap allocation
and a stream read — below the noise floor of measuring it. An orthogonal
compile-time switch would either double what the three-way `add_ut_test` build
has to cover or leave the off path untested, and an untested path that disables
safety checks is worse than no path at all. Revisit only if a profile shows the
branch.

### The bound exists only where wire input drives an allocation

`max_bytes` applies to `vec_field`, `str_field` and `vector_of_records` and
nowhere else, and this is a promise rather than an omission. A fixed-size
field, a `fixed_array_field`, an `array_of_records` and a `c_arr_field` all
have extents fixed by template parameters; nothing a stream says can change how
much they allocate, so there is nothing to bound. Declaring `max_bytes` on one
is a compile error (§3), not a no-op.

**State and data lifecycle.** The feature's entire purpose is a partial-completion
boundary: it moves the failure to *before* the container is resized, so a
rejected length leaves the field's value untouched rather than sized-but-unfilled.
That ordering — validate, then allocate — is the design's one invariant, and it
holds at both sites and in both stream modes. Beyond it there is no persistent
state, no reversibility or replay requirement, and no auditing: a rejected read
propagates a `cast_error` up through the existing `operator|` fold and the
partially-built `struct_field_list` is discarded by the caller, exactly as any
other read failure is today. 046 additionally removes an existing lifecycle
defect (a resized container whose tail is never written, returned as success).

**Error propagation.** Unchanged in mechanism, deliberately. `checked_byte_count`
returns `std::expected<std::size_t, error_reason>`, the read layer's vocabulary;
`read_native` and the records reader convert it to `rw_result` by propagating the
same `error_reason`; `struct_cast_impl` attaches `failed_at` and widens to
`cast_error` at the layer that already does that for every other reason. The new
enumerator therefore crosses exactly the seams the existing ones cross, and no
layer learns anything new about the one below it. The read layer knows a byte
count and a ceiling; it does not know the field's name, and does not need to,
because the enclosing cast step supplies it.

**Naming.** `error_reason::excessive_length`, appended after
`found_contradicting_length` so existing values are unchanged. It is one
enumerator serving both slices, matching the fact that both are one comparison:
in 046 it reads "this length cannot be represented as a byte count", in 047 "this
length exceeds the declared ceiling", and both are true statements about an
excessive length. Rejected: two enumerators (`length_overflow` +
`max_bytes_exceeded`), which would force a caller to handle two reasons for one
condition and would put a distinction in the API that the implementation does not
make. Also rejected: reusing `buffer_exhaustion`, which the spec forbids and
which would conflate "the stream ran out" with "the schema said no".

**Concurrency and ownership.** None arises, and stating it is the point:
everything here is a pure function of a length, a `sizeof`, and a compile-time
constant, called on the stack of a single read. `read_native` owns the container
it resizes for the duration of the call and shares nothing. No static or
thread-local state is introduced, so the helper is trivially reentrant and safe
to call from multiple threads on distinct fields — which is the property that
would break silently if a future version cached anything.

**Reuse.** Uses, rather than reimplements: 045's `pack_options` /
`*_in_pack` / `resolved_options` scan (the bound is a third kind in it, not a
parallel mechanism); `size_type_of` for stripping the const off variable-template
`decltype`s; the `is_X`/`is_X_v`/`X_like` trait-plus-concept triple used
throughout `field_size.hpp`; `std::expected` and the existing `operator|` fold;
`constexpr_memstream` for the compile-time tier. Introduced for reuse:
`checked_byte_count` is the project's only length-to-byte-count conversion after
this lands, and any future site that resizes from a wire length should route
through it — including the seek-based work the spec's Non-Goals anticipate.

**Extension points.** One, and it is the ceiling parameter: `checked_byte_count`
takes the ceiling as a template argument with a widest default, so a future
source of a bound (a stream's remaining extent, a library-wide default — both
Non-Goals *now*) plugs in by supplying a different ceiling and needs no new check
site. The contract that keeps it stable is that the ceiling is a byte count and
the comparison is inclusive. There is deliberately **no** extension point for
per-reason error payloads (the spec fixes `cast_error`'s shape) and none for
user-supplied bound predicates — a ceiling is a number, and a callable there
would put arbitrary user code on the allocation path.

**Build vs. buy.** Nothing to buy. The one component with an off-the-shelf answer
is the checked multiply: `__builtin_mul_overflow` (gcc/clang) and
`std::mul_overflow`-shaped proposals were considered. `__builtin_mul_overflow` is
not available on MSVC, one of the three supported toolchains, and would need a
`#if` ladder plus its own constexpr argument; the division form is portable,
`constexpr` on all three by construction, and — decisively — *also* expresses the
bound comparison, which no overflow builtin does. Building is one line, not a
component. Everything else (the option vocabulary, the classifier) is
project-specific DSL with no external equivalent.

**Abstractions introduced.** Three, each with its forcing problem:
`checked_byte_count` (two call sites must agree on one predicate, and the
predicate must not be expressible as an unguarded multiply); `max_byte_count<N>`
+ `use_default_bound_t` (the pack classifier keys on types, so a bound must have a type
distinct from `byte_count` or a ceiling would be classified as a size);
`bound_in_bytes` (recovers `N` from the type at the one point where "declared" and
"undeclared" collapse to a number). No wrapper type, no bound *policy*, no
`allocation_limiter` interface — all considered and all lacking a second
implementation to justify them.

**Alternatives rejected** (collected; each argued in place above): the size
wrapper `bounded_size_t<...>` (§2); the field wrapper `bounded_field<...>` (§2);
a runtime ceiling on `struct_cast_*` (§2, also spec'd out); per-site duplicated
checks instead of a shared helper (§1); a multiplication-with-overflow-test
instead of a division (§1); two error enumerators (§5); a resolved-bound
`requires`-clause instead of per-descriptor pack concepts (§3); giving both
`read_native` overloads the ceiling parameter (§1); `max_byte_count<SIZE_MAX>` as
the "no bound" value instead of `use_default_bound_t` (§3).

---

## 6. Tests

Per `AGENTS.md`: routing is by constant-evaluability and the compiler enforces it.

**046** — `test/schema/variable_buffer_read_ct.cpp` gains cases driving a length
whose byte count wraps `std::size_t` for a wide element type
(`vec_field<"v", u64, len_from_field<"n">>` with `n` near `SIZE_MAX/8`), asserting
`excessive_length` rather than a resized container. The mirror runtime case goes
in `test/schema/variable_buffer_read.cpp` (GoogleTest, real stream), per the
"new stream-touching code needs both forms" convention. Note the wrap case never
reaches the stream — it fails before the read — so the runtime case is about the
same code path being live in a non-constexpr build, not about stream behaviour.

**047** — a new pair, `test/schema/allocation_bound_read_ct.cpp` (ut) and
`test/schema/allocation_bound_read.cpp` (GoogleTest), covering the 3×3 matrix the
issue fixes: below-bound / at-bound / over-bound × `vec_field` / `str_field` /
`vector_of_records`. Plus, because the ceiling is compared by integer division:
one case where `N` is not a multiple of `sizeof(element)`. A new pair rather than
additions to `variable_buffer_read*` because the tree is organised by what a file
verifies, and this verifies an option, not a container kind.

**Must-not-compile** — two cases appended to
`test/must_not_compile/option_pack_misuse.cpp` as `CASE 3` and `CASE 4`, wired in
`test/must_not_compile/CMakeLists.txt` as `rejects_bound_on_fixed_size_field` and
`rejects_duplicate_bound_option`. They belong in that file because both are the
option pack declining an entry — the same subject the file already documents.
Assert only that the build fails; message text is not portable across the three
toolchains.

`single_header/s2s.hpp` is regenerated in the same commit as 047 (and 046),
since `test/must_not_compile/` and `test/shipped_header/` consume the amalgam.

---

## 7. Documentation

- `docs/schema/index.md`: `max_bytes` in the descriptor table / option list,
  since that table is what `test/schema/` mirrors.
- `docs/schema/size-axis.md`: the substantive entry. This is the page that
  explains where a length comes from, and the bound is a statement about a length,
  so the "off by default" callout belongs here rather than in a page of its own —
  a reader learning `len_from_field` is exactly the reader who needs to be told
  that nothing bounds it unless they say so. Written as an admonition, in neutral
  third person, naming the exposure plainly: a field with no `max_bytes` allocates
  whatever the wire says.
- Any complete program shown must be backed by a `test/doc_examples/` source with
  the `<!-- docs: ... -->` / `// docs-begin` binding; prefer extending an existing
  size-axis example over adding a new one.

---

## 8. Open questions and known gaps

1. **A `vec` alternative inside a `variance` cannot be bounded *explicitly*.**
   `to_field_choice` builds `field<id, T, size, no_constraint<T>{}>` from
   `as_vec<T, len_from_field<"n">>`, which takes a size, not an option pack — so
   union alternatives get the default `use_default_bound` and resize from an unvalidated
   length exactly as today (they do get 046's overflow fix, which is unconditional).
   Since the default ceiling now applies wherever no bound is declared, union
   alternatives are *protected* — they simply cannot be given a ceiling of their
   own. That downgrades this from a hole to an inconvenience: the failure mode is
   a legitimately large union alternative being rejected with no way to raise its
   limit short of the global macro. Extending `as_vec` to an option pack is the
   natural follow-up and is out of scope here; worth a line in the docs so it is
   not discovered the hard way.
2. **`array_of_records` / fixed arrays are untouched** and correctly so — their
   extent is a template parameter, not wire input. Recorded because their absence
   from the boundable set otherwise looks like an omission.
3. **Whether `read_foreign_buffer`'s byteswap needs anything** — it does not; it
   operates on an already-validated, already-filled container. Recorded because it
   is the one buffer path that does not obviously appear in this design.
