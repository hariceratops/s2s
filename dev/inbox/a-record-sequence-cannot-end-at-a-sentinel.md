# A record sequence can end at a count, never at a sentinel

Brief, from a design conversation on 2026-08-30. Not a PRD — the open questions
at the bottom are the interview's agenda. Companion to
`the-size-axis-cannot-say-read-until-a-byte.md`; the two came out of one
discussion and are kept apart because they touch different axes and have
different write-side rules.

## The goal

`vector_of_records` takes its element count from the size axis — a literal, a
sibling field, or a callable. Some formats don't supply one. A GIF image's data
is a run of length-prefixed sub-blocks terminated by a zero-length sub-block;
DNS label lists have the identical shape. The run's length is known only when
you reach the thing that ends it.

The goal is to let a record sequence declare what terminates it instead of how
many elements it has.

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

## Considerations the discussion surfaced

**Read is read-then-test, so the loop always consumes one element more than it
keeps.** For GIF that lands exactly right: the terminator *is* a degenerate
sub-block — a lone zero byte that parses as `size = 0` with no data following —
so consuming it as an element consumes precisely the bytes it occupies and
leaves nothing stranded. Whether that holds for every format worth supporting is
an open question below.

**Growth becomes incremental.** `push_back` per element rather than one
`resize(n)`, which means the allocation bound has to be re-checked after each
element rather than once before the loop, against accumulated
`n * sizeof(element)` — the same memory-footprint denominator already settled on
for bounded vectors of records, and for the same reason: a record carrying
variable-length subfields has no statically known wire size.

**The bound becomes load-bearing rather than defensive.** A corrupt stream that
never yields the sentinel loops until the ceiling stops it, and nothing else
will. This is the same shift the delimiter case undergoes.

**The write side decides the design, and it rules out the obvious spelling.** The
vector holds only real elements, so the engine has to emit the terminator itself
after the last one. A predicate — `[](const sub_block& b){ return b["size"_f] ==
0; }` — cannot do that. A test cannot be run backwards to produce the thing it
accepts. This is the distinction the library already draws between `eq{v}`,
which the write path can satisfy, and `lt{100}`, which it can only check.

**Declaring the terminator as a value doesn't work either.** A `sub_block`
contains a `std::vector`, so it is not a structural type and cannot be a
template parameter.

**What does work is a field and a value.** `until_field_equals<"size", u8{0}>`
— a `fixed_string` id and an integral, both structural. On read it is the test.
On write it is a constructor: default-construct an element, set that field,
write it. Because `data`'s length is driven by `"size"`, that emits exactly one
zero byte. DNS labels have the same shape.

**Running out of stream is not the same failure as a malformed element.** A run
that ends without its sentinel should say so rather than report
`buffer_exhaustion`, which is true but tells the caller nothing about what is
wrong with the file.

## Directions rejected

**A predicate over the element.** The natural spelling, and it fails on write:
the engine must produce a terminator, and a predicate only recognises one.

**A terminator declared as a complete element value.** Not structural whenever
the record holds a `std::vector` or `std::string`, which is the common case and
is true of the motivating example.

**Peeking or ungetting so the terminator stays unconsumed.** The stream concept
has no lookahead — `read(dest, n) -> stream&` and nothing else — and adding one
is the same breaking change to every custom stream that sank read-until-end-of-
stream in the companion brief.

## Questions still open

- Should the terminating element be discarded or kept in the vector? Discarding
  keeps the user's data clean but forces the write path to synthesise. Keeping
  it makes writing trivial but puts a non-data element in the user's vector and
  makes the element count off by one from what the format means.
- Does `until_field_equals` have to name the element's *first* field? The
  write-time construction only produces the right bytes when every later field's
  width is driven by the named one. A terminator whose other fields have
  independent widths would emit a full-size element, not a one-byte sentinel.
  Is that a compile-time check, and what is it checking?
- Does "sentinel not found before the stream ended" get its own `error_reason`?
- Does this belong on `vec_field` of trivials as well, or only on
  `vector_of_records`? A run of trivials ending at a sentinel value is
  expressible as the delimiter feature instead, and having two ways to say it
  would be worse than having one.
- What happens when a sentinel-terminated run is nested inside another one, and
  does the bound apply per run or across the whole nest?
- Is there a format where the sentinel is a *value the schema reads earlier*
  rather than a constant? Nothing in the scan suggested one, but the question
  decides whether the value stays a template parameter.

## Settled elsewhere (2026-09-10)

The companion's requirements interview ran first and closed three of the
questions above. Recorded here so this brief's own interview starts from them
rather than reopening them.

- **Delimited runs of trivials are not this feature's.** `until<>` is accepted
  on `vec_field` as well as `str_field`, so a run of trivials ending at a
  sentinel value is spelled there. The open question "does this belong on
  `vec_field` of trivials as well" is answered no, on the grounds this brief
  already gave: two ways to say one thing is worse than one.
- **The terminating element should be discarded** — a recommendation, not yet
  the user's decision. The reasoning: keeping it moves the cost onto the caller,
  who must remember to append a terminator by hand on write, and a caller who
  forgets writes a malformed file with no error raised anywhere. That is the
  same silent-corruption class the companion's "a value containing the
  delimiter must fail to write" rule exists to prevent, and the library already
  distinguishes constraints it can satisfy (`eq{v}`) from ones it can only
  check (`lt{100}`). Discarding forces the write path to synthesise, which is
  what makes the open question below — whether `until_field_equals` must name
  the element's first field — a real compile-time check that needs a precise
  rule.
- **Acceptance criteria carried forward:** GIF sub-block runs and DNS label
  lists, both round-tripping. Named during the companion's interview as this
  feature's witnesses, not that one's.

Still open and untouched by that interview: whether the named field must be the
element's first, and what the compile-time check is checking; a distinct
`error_reason` for a run that ends without its sentinel; nesting, and whether
the bound applies per run or across the nest; whether any format supplies the
sentinel as a value read earlier.

One correction to this brief's own reasoning, from the same interview: the
rejection of "peeking or ungetting so the terminator stays unconsumed" rests on
a false premise. Adding a lookahead operation to the stream concept is
**additive, not breaking** — a capability concept plus `if constexpr` dispatch
leaves every existing custom stream compiling, on a fallback path. The direction
may still be wrong for this feature, but it has to be re-argued on its merits
rather than dismissed as a breaking change. Note also that `peek` is the wrong
primitive regardless: `std::istream::peek()` pays the same per-call sentry cost
as `read`, and peeking K bytes at once has no `std::istream` spelling that does
not require a seek.
