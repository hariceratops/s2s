# Union alternatives carry no options

## Goal

A `variance` alternative should be able to carry the same trailing options any
other field carries — a constraint, and now an allocation bound. Today it can
carry neither, and the reason is not a decision anyone made.

`include/field/field.hpp:80`:

```cpp
template <fixed_string id, typename T, auto size>
struct to_field_choice {
  using field_choice = field<id, T, size, no_constraint<T>{}>;
};
```

The fourth argument is hardcoded. Every alternative of every union is built with
`no_constraint`, so a `match_case` whose payload should be validated cannot say
so. With `max_bytes` landing as a fifth parameter (see
`dev/design/unbounded-resize-from-wire-length.md`), a vector alternative will
likewise take the default bound and have no way to state its own.

The problem behind it: the tag-to-field pipeline transports exactly two things
per alternative. `to_field_choices` zips `std::variant<types...>` against
`size_choices_t<sizes...>` positionally, and there is no third list to zip, so
a type tag has nowhere to put anything that is not a type or a size. The type
tags themselves (`as_trivial`, `as_vec`, `as_string`, …) expose exactly
`::type` and `::size` for the same reason.

Every other descriptor gained an order-independent option pack when
`dev/specs/schema-api-verbosity.md` landed. Union alternatives were left out of
that work deliberately — the spec froze the variance API pending a separate
question — but the exclusion predates it and is wider than anyone stated.

## Considerations surfaced

**It is mechanical, not hard.** `as_vec` and friends grow an options member;
`bound_from_type_condition` and `bound_choices_from_type_conditions` mirror the
size versions in `type_deduction/utils/helper.hpp` almost line for line; the
three deduction guides (`type.hpp`, `switch.hpp`, `ladder.hpp`) grow a
`::bounds` beside `::sizes`; and `to_field_choices` deduces three packs instead
of two, which works — they are three separate type arguments, each yielding its
own pack. Roughly thirty lines, nearly all copied from what is already there.

**It is one gap, not two.** Framing this as "let `as_vec` take a bound" gets the
same code and misses that constraints have been silently unavailable the whole
time. Framing it as "give alternatives an option pack" picks up both, and lines
up with the machinery `field_descriptors.hpp` already has rather than inventing
a parallel one.

**It hangs off the piece that is already unsettled.** All of this rides on
`size_choices_t`, which is what `dev/issues/027-is-selectable-size-trait-reads-wrong-trait.md`
is open about: `size_choices` is not user-declarable, and while 044 re-derived
its trait onto values, whether that shape survives contact with a *declarable*
`size_choices` is explicitly unresolved. Adding a parallel `bound_choices_t`
beside it now doubles whatever 027 eventually has to reconsider. That is the
real reason to wait, and a better one than scope.

**The bound gap is currently an inconvenience rather than a hole.** Because the
allocation ceiling defaults to `default_max_bytes` rather than to unbounded, a
vector alternative is protected even though it cannot declare its own limit.
What it cannot do is raise that limit for itself: a legitimately large union
alternative is rejected with no recourse short of the global
`S2S_DEFAULT_MAX_BYTES` macro, which moves every other default with it. The
constraint gap has no such mitigation — an alternative simply cannot be
validated.

## Directions rejected

**Extending `as_vec` alone to take a bound.** Same work, narrower result. It
would leave constraints unavailable and leave a second reason to come back here
later.

**Doing it inside the `max_bytes` work.** That spec's slices are bounded by two
resize sites and a classifier; adding a third parallel choices list to the union
machinery is a different piece of work with a different risk, and it would drag
027 into a change that currently does not touch it.

## Open questions

- Does this wait on 027, or does 027 wait on this? A declarable `size_choices`
  and a per-alternative option pack are the same machinery seen from two sides,
  and doing either first may settle the other.
- Should an alternative's options be spelled on the type tag (`as_vec<u8,
  len_from_field<"n">, max_bytes<4096>>`) or on the `match_case` beside it? The
  tag is where the size already lives, which argues for the tag; but the tag is
  a type and the option pack is values, so the two do not compose as neatly as
  they do on a descriptor.
- Does a per-alternative constraint interact with the write path's verify step,
  which currently checks `T::constraint_checker` on a field it assumes has one
  meaningful constraint?
- Is there any alternative kind for which an option pack is meaningless, the way
  a bound is meaningless on a fixed-size field? `as_struct` and
  `as_arr_of_records` have no wire-driven allocation, so at least the bound half
  of the pack should be rejected there by the same concept mechanism.
