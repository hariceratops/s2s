# Errors

Both directions report failures the same way. `cast_error` carries the reason
and the name of the field it happened at.

```cpp
enum error_reason {
  buffer_exhaustion,
  validation_failure,
  type_deduction_failure,
  found_contradicting_length,
  excessive_length,
  delimiter_not_found,
  found_delimiter_in_value,
  sentinel_not_found,
  found_sentinel_in_sequence
};

struct cast_error {
  error_reason failure_reason;
  std::string_view failed_at;
};
```

A read can fail in four ways: a field value fails validation, the input stream
is exhausted, type deduction fails while reading into a union, or a
length-prefixed field claims more memory than it is allowed to allocate.

`excessive_length` is the odd one, and the difference matters when reading a
foreign stream. Every other reason is reported *during* a read;
`excessive_length` fires **before** one. `buffer_exhaustion` means the stream
ran dry partway through; `excessive_length` means the length was never
satisfiable and nothing was allocated for it. See
[Allocation limits](reading.md#allocation-limits) for the ceiling it reports
against and how to change it.

A delimited field (`until<d>`, see [The size axis](schema/size-axis.md)) can
fail a fifth way while reading: `delimiter_not_found`, when its `max_bytes`
bound is reached before the delimiter turns up. It is not `buffer_exhaustion`
under another name — `buffer_exhaustion` means the stream ran dry;
`delimiter_not_found` means bytes kept arriving and none of them was the
delimiter. Truncated and corrupt are different facts about a file, and a
caller can act differently on each.

A sentinel-terminated record run (`until_field_equals<field, value>`, see
[The size axis](schema/size-axis.md)) can fail a sixth way while reading:
`sentinel_not_found`, when its `max_bytes` bound is reached before a sentinel
element turns up. As with `delimiter_not_found`, it is not `buffer_exhaustion`
under another name: the stream did not run dry, it kept supplying elements and
none of them was the sentinel.

A byte-order marker that matches no case is a `validation_failure`, at the
announcing field's id — not a `type_deduction_failure`, though it is a deduction
that failed. On the type axis a failed deduction means the reader cannot proceed
at all: it does not know how many bytes to consume next or what to consume them
into. On the byte-order axis it means the file is not this format, which is a
magic mismatch, and a magic mismatch is a `validation_failure` everywhere else
in the library. See [The byte-order axis](schema/byte-order-axis.md).

A write can fail in the first three ways plus `found_contradicting_length`,
which means two parts of the struct imply different lengths for the same data —
a cross-field disagreement rather than a value that is wrong on its own terms.
`excessive_length` is read-only: a write's container is one the caller already
holds, so nothing is being allocated from an untrusted number.

A write can also fail with `found_delimiter_in_value`, when a delimited field's
value contains its own delimiter byte and so cannot round-trip: written whole,
it would read back short with no error raised anywhere. It is not folded into
`validation_failure` — the author has not broken a constraint they wrote, they
have hit a rule of the size form itself, and no other size form has a content
rule at all. The check runs before any byte of the field is emitted, so a
rejected value leaves the stream untouched.

A write can also fail with `found_sentinel_in_sequence`, when an element of a
sentinel-terminated run would be written as the sentinel and so would read back
as the end of the run, silently dropping every element after it. It is not
`validation_failure` for the same reason as above: the rule belongs to the
termination form, not to a constraint the author wrote. The check runs per
element, so earlier elements' bytes are already on the stream when it fires, and
nothing is rolled back; the offending element's own bytes are not.

Which check produces which reason is tabulated per direction:
[Read errors](reading.md#read-errors) and
[What is checked at write time](writing.md#what-is-checked-at-write-time).

## What `failed_at` names

`failed_at` is always a field id that appears in the schema passed to the cast.
For a failure inside a nested record it names the **outermost** record field,
not the inner one — a validation failure two levels down inside
`struct_field<"header", ...>` reports `"header"`.

That is a deliberate limitation rather than an oversight: `cast_error` carries
one name, so a nesting chain and a cross-field contradiction cannot both be
expressed. The project's development backlog records the design discussion under
"Error ergonomics", including why a shared-length contradiction names the length
field but not the dependent that disagreed with it.
