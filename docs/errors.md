# Errors

Both directions report failures the same way. `cast_error` carries the reason
and the name of the field it happened at.

```cpp
enum error_reason {
  buffer_exhaustion,
  validation_failure,
  type_deduction_failure,
  found_contradicting_length,
  excessive_length
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

A write can fail in the first three ways plus `found_contradicting_length`,
which means two parts of the struct imply different lengths for the same data —
a cross-field disagreement rather than a value that is wrong on its own terms.
`excessive_length` is read-only: a write's container is one the caller already
holds, so nothing is being allocated from an untrusted number.

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
