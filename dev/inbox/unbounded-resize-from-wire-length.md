# A length read off the wire resizes a container with no bound

## Goal

A stream produced by another tool — corrupt, truncated, or hostile — should
fail to parse as a `cast_error` the caller can inspect, not as a `bad_alloc` or
an OOM kill. Today it does the latter, because both read paths for
variable-sized fields take a length that came out of the stream and hand it
straight to `resize`.

`include/field_read/read_impl.hpp:51`, for vectors and strings of trivials:

```cpp
template <variable_sized_buffer_like T, input_stream_like stream>
constexpr auto read_native(stream& s, T& obj, std::size_t len_to_read) -> rw_result {
  obj.resize(len_to_read);
```

`include/field_read/field_reader.hpp:153`, for vectors of records:

```cpp
auto len_to_read = deduce_field_size<field_size>{}(field_list);
field.value.resize(len_to_read);
```

Nothing between the stream and the allocation checks `len_to_read` against how
many bytes the stream actually has left. A `len_from_field<"n">` sized field
whose `n` is a corrupt `u32` allocates up to 4 GB — and with `vector_of_records`,
up to 4 G *records* — before a single byte of payload is read.

The problem behind it: the buffer-exhaustion check that exists today runs after
the fact. `read_native_impl` reports `buffer_exhaustion` when the read comes up
short, which is the right error arrived at the wrong way round — the allocation
has already happened by the time anyone notices the stream cannot satisfy it.

## Considerations surfaced

**Contracts are not the mechanism, despite the C++26 port making them
available.** A length that disagrees with the stream is not a precondition
violation on this library's API — it is exactly the input the library exists to
handle, and the caller has done nothing wrong by passing a file someone else
produced. Contracts terminate or run a violation handler rather than return, so
expressing this as a contract turns a recoverable parse failure into an abort in
the one case that matters most. Contracts would suit the internal invariants
around this code — that `deduce_field_size` is never reached before its source
field is populated, say — but that is separate work and does not close this.

**Non-seekable streams cannot answer "bytes remaining".** A seekable stream can
be asked directly, and that is the tightest possible bound. A pipe or socket
cannot, so a stream-derived bound does not cover every case and something else
has to backstop it.

**There is an adjacent overflow.** `read_impl.hpp:60` computes
`len_to_read * sizeof(T{}[0])` for the non-constexpr path. A large element type
and a large length can wrap `std::size_t`, producing a byte count smaller than
the number of elements resized for. Same code path, same fix window.

**This meets the schema API work at `cast_error`.**
`dev/inbox/schema-api-verbosity.md` wants derived fields hidden from the struct
API, which is only acceptable if a failed parse can still show what value caused
the failure — and `cast_error` today carries `failure_reason` and `failed_at`
only. Both pieces of work want the same enrichment and should agree on its
shape rather than each growing their own.

**The compile-time tier can cover this.** `constexpr_memstream` knows its own
extent, so the rejection path is testable without a real file, under both
execution modes.

## Directions rejected

**Leaving it to the post-read `buffer_exhaustion` check.** It is the correct
error and it does fire — after the allocation that is the actual harm. Ordering
is the whole issue.

**Treating it as a contract violation.** See above.

## Open questions

- What backstops a non-seekable stream? A configurable ceiling on a single
  field's allocation is the obvious candidate, but its default has to be picked,
  and a default that is generous enough to be unsurprising may be too generous
  to be protective.
- Is a length that cannot possibly be satisfied a new `error_reason`, or a reuse
  of `buffer_exhaustion`? It is arguably a different fact from a stream that ran
  out mid-read.
- Does the bound belong in `read_native` and `read_field` separately, or at the
  one place `deduce_field_size` produces a length for a variable-sized field?
  The latter is one site rather than two, but it is further from the stream that
  knows the answer.
- Does an incremental `resize`-and-fill loop remove the need for a bound
  entirely, by never allocating more than has been read? That trades the check
  for reallocation cost on every large field, and it is not obvious which is
  worse.
