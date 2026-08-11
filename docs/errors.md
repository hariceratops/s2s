# Errors

Both directions report failures the same way. `cast_error` carries the reason
and the name of the field it happened at.

```cpp
enum error_reason {
  buffer_exhaustion,
  validation_failure,
  type_deduction_failure,
  found_contradicting_length
};

struct cast_error {
  error_reason failure_reason;
  std::string_view failed_at;
};
```

A read can fail in three ways: a field value fails validation, the input stream
is exhausted, or type deduction fails while reading into a union.

A write can fail in those same three ways plus `found_contradicting_length`,
which means two parts of the struct imply different lengths for the same data —
a cross-field disagreement rather than a value that is wrong on its own terms.

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
