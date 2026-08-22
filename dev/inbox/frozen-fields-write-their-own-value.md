# A field pinned to one value should write itself

Brief, from a design conversation on 2026-08-21. Not a PRD — the open
questions at the bottom are the agenda for the interview, not leftovers.

## The goal

A field constrained to `eq{v}` has exactly one legal value, and the library
knows it at compile time. Today the user still has to supply it: they write
`obj["magic_str"_f] = s2s::fixed_string<3>("GIF")` into a slot where "GIF" is
the only thing that could ever have gone, and if they get it wrong — or forget
— the write fails at run time with a `validation_failure` naming the field.
That is a chore dressed up as a check. The value should come off the
constraint's own template parameter and go to the stream, and the setter
should stop being offered at all.

The problem behind it is that the schema already states the value and the
struct makes the user state it a second time. Every restatement is a chance to
disagree with the schema, and none of them can ever be right in a way the
schema wasn't already. `magic_string`, `magic_number` and `magic_byte_array`
are the obvious cases, but nothing about them is special — they are aliases
that happen to spell `eq`, and a `basic_field<"tag", u8, 1_B, eq{7}>` is in
exactly the same position.

This is the same move the library already made for length fields, and for the
same reason. `docs/writing.md` says a length is "not data anyone supplies; it
is a consequence of the message" — a magic value is not data anyone supplies
either; it is a consequence of the format.

## Considerations the discussion surfaced

**The gate is `eq`-constrained *and* fixed-sized.** Keying off the constraint
type — `decltype(constraint_checker)` being a specialization of `eq` — rather
than off the descriptor alias covers the three magic aliases and a hand-spelled
`eq{}` in one trait, with no special-casing. Conjoining it with
`fixed_sized_field_like` then drops the cases that are not routine and not
wanted: struct fields, arrays and vectors of records. It also drops one that
would otherwise need real thought — `str_field<"s", eq{...}>`, where freezing
the value makes the length statically known and collides with the derived-length
machinery. Only `eq` qualifies among the constraints; `neq`, `any_of` and the
range constraints admit more than one value and pin nothing.

**Hiding the setter, not the getter.** Two precedents exist in
`field_list.hpp`. Length targets have no `operator[]` overload at all,
because the reference `operator[]` hands back could be used to mutate a
container behind any setter's back. Discriminants keep a readable overload
that returns `const&` even from a mutable object, so an assignment fails as
assign-to-const rather than as a wall of unsatisfied-constraint output. The
discriminant shape is the right one here: a fixed-size magic value cannot be
laundered through a const reference, reading back what was parsed is
legitimate, and the diagnostic is better. In `test/schema/magic_write.cpp`
that is the difference between deleting four setter lines and also having to
rewrite every assertion in `expect_matches_populated`.

**Unlike a length target, this is intrinsic to the field.** Length-derived and
discriminant-derived are cross-field properties, which is why they need
`dep_vec`s in `field_list_metadata.hpp` computed over the whole pack. Frozen-ness
is visible in the field type alone, so `operator[]` can ask the type it already
looked up and the metadata needs no third list.

**Frozen on write, still validated on read.** Reading a file whose magic bytes
are wrong must still fail — `struct_cast_impl.hpp` is untouched. The asymmetry
is the intent and should be stated as such, not arrived at by omission.

**A behaviour is deleted, not repaired.** `MagicWrite.RejectsWrongMagicString`
and `RejectsWrongMagicNumberInBothByteOrders` exist to check a failure this
change makes unreachable. After it, the write path can never emit
`validation_failure` for a frozen field. Those tests get deleted.

**Ordering against derived targets.** A frozen field can also be the target of
some other field's `len_from_field`. `field_writer.hpp` must keep its
`is_derived_target_v` branch first and add the frozen write as an `else`: the
derived path already validates the derived value against the constraint, which
is the correct behaviour for a field that is both.

**The stored constant is not necessarily the field's type.**
`magic_number<"m", u64, 8_B, 0xdead>` builds `eq<int>` over a `u64` field,
because `magic_number` takes `auto expected` and `eq`'s CTAD deduces from the
literal. The trait has to match `eq<U>` for any `U`, and the write has to cast
to `field_type` — the same care `derive_value` already takes about a declared
width being narrower than the value it is handed.

**Documentation moves in lockstep.** `docs/writing.md` and
`test/doc_examples/guide_writing_example.cpp` are diffed against each other by
`doc_examples_match`. The worked example there sets `record["marker"_f]`
explicitly; dropping that line makes the marker join `message_length` as a
field nobody assigns, which is the point that section is already making.

## Directions rejected

**An explicit opt-in marker** — some `frozen<...>` option in the trailing
pack. Rejected: the schema already says `eq`, and a second spelling of the
same fact would let the two disagree. The whole complaint is that the value is
stated twice.

**Hiding frozen fields from `operator[]` entirely**, as length targets are
hidden. Rejected above — costs the read assertions and gives a worse
diagnostic, for a mutation risk that fixed-size fields do not have.

**Extending the freeze to struct fields and containers of records.** Rejected
as not a routine use case, and it drags in questions about what an `eq` over a
whole record even means for the write path.

**Deriving frozen-ness in the metadata tables** alongside the two existing
`dep_vec`s. Rejected as unnecessary work — the property is on the field type.

## Open questions

- What happens to a frozen field inside `maybe<>`? The optional's presence is
  still user-supplied while the value is not, so the setter would have to
  accept engagement but not content — or the field stops being settable and
  presence becomes unreachable. `field_writer.hpp` already unwraps to the base
  field type and runs its constraint explicitly, so the write side works
  either way; the question is what `operator[]` should offer.
- Same question for an alternative inside a `variance`. An alternative's
  constraint now travels on its type tag and there is no `operator[]` for an
  alternative at all — how a caller reaches a held alternative is recorded as
  unsettled in `field_list.hpp`. Does freezing have anything to say there, or
  does it wait on that?
- Should attempting the assignment be pinned down by a `must_not_compile`
  test, and if so is assign-to-const a stable enough diagnostic to assert on?
  `test/must_not_compile/` matches against expected output.
- Does the frozen value still need to land in `field.value` on read? Keeping
  it means a parsed struct and a default-constructed one differ in a field the
  user cannot set, which is either useful or confusing depending on whether
  anyone reads it back.
- Is `eq` the only constraint that should freeze, or does a single-element
  `any_of` want the same treatment? It pins one value just as firmly, and the
  answer decides whether the trait is about `eq` specifically or about
  "constraints with exactly one satisfying value".
