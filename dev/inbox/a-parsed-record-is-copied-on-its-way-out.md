# A parsed record is copied on its way out

Brief, from a design conversation on 2026-11-07. Not a PRD — the open questions
at the bottom are the interview's agenda.

## The goal

s2s's pitch is that the parser is generated at compile time. A schema should
therefore cost what a hand-written read of the same struct costs. On the read
path it does not:

- `struct_cast` builds the record in a local, then copies it into the returned
  `std::expected`.
- Every nested record is built in its own local and copied twice on the way up.
- An optional field copies its value out of a staging field.

For a record that holds `std::vector`s, each of those copies is a deep copy with
its own allocations.

The developer's words: "we need a new brief optimizations to get as much zero
cost wrt copying", and "i cant ship a lib which is suboptimal, we will bench mark
schemas against handwritten reads and writes and later find places for
optimization".

This is v1 work, due January 2027. v1's refactoring is bounded: "refactoring
stops with benchmarks, performance tuning, namesweep and dead code removal. if
needed the bareminimum changes to introduced as much value based
metaprogramming". The schema brief (`a-schema-cannot-be-a-plain-struct.md`)
records the v1 rule for what counts as "needed": new code that computes data,
code already open for v1 work, and code a benchmark shows is the problem.

The goal is a read path within 3–5% of a hand-written read on all three
supported compilers, shown by benchmarks that run in CTest.

## Considerations the discussion surfaced

**What is settled.**

- **Benchmark inputs:**
  - GIF files from the web, supplied by the developer;
  - synthesised IPv4 packets, CAN frames and EtherCAT frames, "and more", at
    1–10 records per message.

  Nesting goes up to level 3.
- **Streams:** all of them.
- **Harness:** Google Benchmark, run in CTest. The benchmarks get their own
  CTest label, and each passes or fails on the median of repeated runs against
  the 3–5% margin.
- **Pass criterion:** within 3–5% of an optimised hand-written read, on gcc,
  clang and msvc.
- **Fairness:** standard facilities such as `std::byteswap` are allowed on both
  sides. Explicit CPU intrinsics in the hand-written read are "unfair
  competition".
- **The fix:** v1 passes storage down to nested readers.
- **The generator yields by value:** "returning references is nasty wrt
  lifetimes".
- **The view's byte-by-byte staging loop** is a v1 experiment.
- **On failure,** the caller sees either a complete record or a `cast_error`, as
  today (see below). This holds for every API that returns `expected`.

**Evidence: what the top-level return costs today, measured.** The experiment is
at `dev/archive/nrvo_experiment/nrvo.cpp`. It runs as `g++-14 -std=c++23 -O2
nrvo.cpp` (or `clang++-19`), with `-fno-elide-constructors` added for the
worst-case rows. It uses a record type holding a 1000-element
`std::vector<int>` that counts its own copies and moves, and compares two
return shapes:

- **Today's shape**, as in `struct_cast_impl.hpp:21-49`: a local `S` is read
  into, then `return ok ? R(field_list) : std::unexpected(...)`.
- **The in-place shape:** `R r{std::in_place}; if(!read_into(*r, ...)) r =
  std::unexpected(err); return r;`

The results were identical on g++-14 and clang++-19, at both `-O0` and `-O2`:

| Shape | Elision | Success | Failure |
|---|---|---|---|
| Today | on | 1 copy | 0 copies, 0 moves |
| Today | `-fno-elide-constructors` | 1 copy | 0 copies, 0 moves |
| In place | on | 0 copies, 0 moves | 0 copies, 0 moves |
| In place | `-fno-elide-constructors` | 1 move | 0 copies, 0 moves |

MSVC was not tested; the three-compiler benchmark covers it. The experiment
models one level. Nesting is reasoned from the code below, not measured.

**What the evidence says.**

- **Today, elision never happens.** The record already exists as a separate
  local before the `expected` is built, and a copy from an existing object
  cannot be elided. The conditional and `R(lvalue)` also block the implicit
  move. So every record, at every level, is copied once into its `expected`.
- **A nested record pays a second copy per level,** at `field_reader.hpp:243`
  (`field.value = *res`).
- **The public API adds nothing on top.** `include/api/struct_cast.hpp` returns
  `struct_cast_impl<T, stream>{}(s, order)` as a prvalue, which C++17 guarantees
  to elide.
- **So the fix has two parts, and v1 needs both:**
  - one named `R` at the top, read in place and returned by name: zero copies
    with NRVO, one move at worst;
  - parent storage passed down to nested readers, which removes the per-level
    copy at `:243`.

  The table is evidence for the first part. The shape of the second is still
  open.

**Caveats on the in-place shape.**

- NRVO is permitted, not guaranteed. Only prvalue elision is mandatory. The
  fallback is an implicit move, which is what the `-fno-elide-constructors` row
  shows.
- Constant evaluation never elides, so a move happens there. That doesn't
  affect runtime, and moves are constexpr.
- The same named `r` has to be returned on every path.

**The half-written destination is settled for every API that returns
`expected`.** On a failed read, the in-place shape assigns `r =
std::unexpected(err)`. That destroys the partly filled `S`, freeing its
allocations, and builds the error in the same storage. So a caller of
`struct_cast`, or of the generator, sees either a complete record or a
`cast_error` naming the field, exactly as today. Nested readers that write into a
parent leave it partly filled, but the parent lives inside the top-level `r` and
is destroyed with it.

The question comes back only for an API that writes into storage the caller
keeps: a future `struct_cast_into(dest)`, or the annotated front end. Both are
post-v1, and the schema brief records the possible contracts.

**The copies on the read path, located.**

- **`cast/struct_cast_impl.hpp:49`.** `R(field_list)` copies the whole record.
  This is the measured row above.
- **`field_reader.hpp:238-243`, nested record.** One copy into the nested
  `expected`, then `field.value = *res;` copies again. A field at depth *d* is
  copied about 2*d* + 1 times, so about seven times at depth 3.
- **`field_reader.hpp:265-270`, optional.** Read into a local, then copied in.
- **`field_reader.hpp:125-131`, array of records.** A temporary per element, two
  copies, then a move.
- **`field_reader.hpp:182-184`, vector of records.** `resize(len)`
  default-constructs every element; each one then pays the read, two copies and
  a move.
- **`field_reader.hpp:208-219`, sentinel-terminated run.** Two copies, then a
  move into `push_back`.
- **`field_reader.hpp:326-338`, union alternative.** Read into a local, then
  moved. A record alternative pays the two copies first.
- **`field_reader.hpp:294`, byte-order announcing record.** Goes through the
  nested-record reader, so it pays the same two copies.

Scalar and buffer fields read straight into their storage
(`read_impl.hpp:55-61`). A foreign-order buffer is swapped in place
(`read_impl.hpp:160`).

**The write path is already copy-free.** Every `write_field` holds `const
field_type&` (`field_writer.hpp:28-401`), and `stream_cast_impl` takes `const S&`
(`stream_cast_impl.hpp:20`). The only values it creates are:

- a byte-swapped scalar temporary (`write_impl.hpp:57`);
- a derived length or discriminant;
- one synthesised terminator per sentinel-terminated run
  (`field_writer.hpp:239`).

**Passing storage down: the candidates.** Below the top level, each nested reader
could be handed a reference to its destination:

- `field.value` for a nested record;
- `field.value[count]` for arrays and vectors;
- `emplace_back()`, popped on the sentinel, for sentinel runs;
- the engaged value after `emplace()` for optionals;
- `emplace<idx>()` for unions.

This could also mean `struct_cast_impl` changes from "build and return" to "parse
into this", with the entry points wrapping it. It is the same seam the schema
brief needs for parsing into a user's struct.

Under the v1 rule this is not a value-based change. It changes who owns storage,
and the readers still select field kinds by type. It is performance work, which
v1 already includes.

**Does a benchmark matter when inspection already shows the copies?** The
coordinator recommends yes; this is not decided. Inspection shows that copies
exist. Only a benchmark shows whether the fix brings s2s within 3–5%. Copies are
not the only cost: there are also the `constexpr_stream` staging, constraint
checks, and the per-field error path. The benchmark also guards against
regressions, and it triggers the v1 rule's third case. So the run after the fix
is the real test.

The coordinator also recommends that the generator's hand-written twin allocate
a fresh record per item, as the generator does.

**The benchmark inputs lean on bitfields.**

- **IPv4** packs version and IHL into one byte, and IHL gives the options
  length.
- **CAN**, in a byte-aligned form such as SocketCAN's `can_frame`, packs the
  identifier and its flags into a 32-bit word, next to the length code.
- **EtherCAT** has an 11-bit length and a 4-bit type in its header. Each datagram
  carries an 11-bit length and an "M" bit meaning another datagram follows.
- **GIF's** descriptors have packed bytes.

So v1's bitfields have to land first. These inputs also exercise the packed-byte
brief's open questions: bit members as length sources, and a bit member that
ends a sequence.

**Raw CAN on the wire is out of scope.** Bit-stuffed CAN is bit-cursor territory,
which the packed-byte brief rejects. The synthesised CAN input should be a
byte-aligned capture or driver format.

**GIF is the copy showcase.** Image data and extensions are runs of sub-blocks of
up to 255 bytes. A several-MB file has tens of thousands of them, each going
through the per-element temporary-and-two-copies path. The LZW payload stays as
bytes.

The coordinator suggests about ten files, spanning:

- a tiny single frame and a large single frame;
- 100+ frames;
- interlaced images and local colour tables;
- comment and NETSCAPE extensions;
- one file of several MB.

**Small messages measure something else.** At 1–10 records, per-call overhead
dominates: the fold over fields, constraint checks, and the error path. Both ends
are in the set for that reason.

**Timing in CTest: settled as a separate label, with medians.** A 3–5% threshold
sits close to run-to-run noise, and `AGENTS.md` holds that a green `ctest` means
something. The answer is two-fold:

- A label of their own, the way `ubsan` uses `run_time`, keeps the benchmarks
  out of the default run.
- Comparing medians over repeated runs, not a single run, keeps noise from
  deciding a pass.

The repetition count and the machines are still open. There is no CI, and MSVC
needs a Windows machine.

**Allocation counts are a diagnostic.** Every deep copy of a container allocates,
and a replaced global `operator new` gives an exact count. That explains a
timing gap even within the margin. The firmware brief needs the same counter.

**The generator yields by value, which rules out storage reuse.** What has to
avoid the copy is getting the record out of the coroutine frame. As I read P2502,
`co_yield` of a named local copies it unless it is moved. Each record comes from
`struct_cast`, which no longer copies on the way out once the in-place shape is
in. The lazy-records brief (`a-file-of-records-must-be-parsed-all-at-once.md`)
predates this decision and doesn't record it yet.

**The view's staging loop is an experiment.** The view's by-value return is its
documented contract (`docs/views.md:10`). Its `unchecked_byte_source::read`
(`view/struct_view.hpp:46-51`) copies one byte at a time into a staging array,
which `read_native_impl` then bit-casts (`read_impl.hpp:36-53`). That loop keeps
the view working in constant evaluation. The twin to compare against is `memcpy`
plus `std::byteswap`.

**The constexpr-stream arm stages every read, at runtime too**
(`read_impl.hpp:36-53,82-90`, `write_impl.hpp:15-22,39-47`). "All streams" puts it
in the benchmark set.

**User callables can copy.** `compute_impl`
(`computation_from_fields_impl.hpp:60-62`) passes fields by `const&`. A callable
that takes one by value copies it itself.

**The archive gives a starting shape.** `dev/archive/stream_benchmarks/` uses
Google Benchmark to time stream primitives over a file and a `std::spanstream`.
It has no schema and no CMake wiring. `dev/archive/nrvo_experiment/` holds the
return-shape evidence above.

**Hand-written twins are real work.** A fair GIF twin is a hand-written GIF
structure reader.

## Directions rejected

**Optimising before measuring.**

**Local moves only.** The developer chose to pass storage down. The evidence
shows that a top-level change alone leaves the per-level copy.

**Yielding by reference, or into caller storage reused across yields.** Rejected
for lifetimes.

**Making the view zero-copy.** Its copy is the documented contract.

**CPU intrinsics in the hand-written twin.**

**Raw bit-stuffed CAN as an input.**

**Returning a different object on the error path.** It defeats NRVO.

**Benchmarks in the default `ctest` run, or a pass decided by a single run.**
They get their own label, and medians decide.

## Questions still open

- Does the benchmark run before the fix matter, or only the run after it? The
  coordinator recommends after, plus regression guarding.
- Does the generator's twin allocate a fresh record per item? The coordinator
  recommends yes.
- How many repetitions does each median take?
- Who runs the three-compiler comparison, and on which machines, given there is
  no CI and MSVC needs Windows?
- Below the top level, what shape does passing storage down take?
- Which byte-aligned CAN representation is used? What does "and more" cover?
- Do the benchmarks wait for bitfields?
