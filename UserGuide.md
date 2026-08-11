# User Guide [Work in Progress]
Struct
Reading from stream
    Trivial
    Array of trivials
    Array of records
    Length prefixed vector of trivials
    Length prefixed vector of records
    Const sized strings
    Length prefixed strings
    Optionals
    Unions
    Magic strings
    Magic numbers
Writing to stream
    A worked example
    Derived fields are read-only
    What is checked at write time
    Write-once, fail-fast
Using custom stream
Validating field members
Error Handling


## The struct
```cpp
template <typename... fields>
struct struct_field_list { /* implementation */ };
```
A struct_field_list datatype is a meta-struct with a map-like interface to access
a field member for read or write operation. The operator[] when provided with "<field_name>"_f
as key, returns a reference or const reference to the member with name <field_name>

Accessing field not present in the struct_field_list(the "map") will result
in compilation error for field lookup failure, since a check is enforced via concepts 

```cpp
template <typename field_accessor, typename field_lookup = /* field_lookup metafunction */>
    requires /* field_lookup success */
auto& operator[](field_accessor);

template <typename field_accessor, typename field_lookup = /* field_lookup metafunction */>
    requires /* field_lookup success */
const auto& operator[](field_accessor);
```


## Schema

A schema is a type. You declare it once as a `struct_field_list` of field
descriptors, and that single declaration drives both directions: `struct_cast`
reads a stream into it, `struct_write` writes it back out.

Nothing in this section belongs to one direction. Field descriptors are the
schema language, not a read-side feature — where the two directions genuinely
differ is in what they *do* with a declaration, and that is covered in
[Reading](#reading) and [Writing](#writing).

### The four axes

Every field declares up to four things.

| Axis | What it answers | Required |
|---|---|---|
| `id` | what the field is called | always |
| `type` | which C++ type holds the value | always |
| `size` | how many bytes it occupies on the wire | unless the type implies it |
| `constraint` | which values are legal | no — defaults to `no_constraint` |

<!-- docs: test/single_header/guide_schema_example.cpp -->
```cpp
#include "s2s.hpp"

#include <sstream>
#include <vector>

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

using packet =
  s2s::struct_field_list<
    // id         type    size                              constraint
    s2s::magic_string<"magic", "PKT">,
    s2s::basic_field<"version", u16, s2s::field_size<s2s::fixed<2>>, s2s::any_of{u16{1}, u16{2}}>,
    s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::vec_field<"payload", u8, s2s::field_size<s2s::len_from_field<"len">>>
  >;

auto main() -> int {
  packet obj{};
  obj["magic"_f] = s2s::fixed_string<3>("PKT");
  obj["version"_f] = u16{2};
  obj["payload"_f] = std::vector<u8>{0xde, 0xad, 0xbe, 0xef};

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  if(const auto written = s2s::struct_write_be<packet>(stream, obj); !written)
    return 1;

  const auto back = s2s::struct_cast_be<packet>(stream);
  if(!back)
    return 1;

  // "len" was never assigned; it is the size axis of "payload" resolved.
  return (*back)["len"_f] == 4 && (*back)["version"_f] == 2 ? 0 : 1;
}
```

**`id`** is a string literal and is how you reach the field:
`obj["version"_f]`, using the `_f` literal from `s2s_literals`. Ids are unique
within a `struct_field_list` — a duplicate is a compile error, and so is looking
up an id the schema does not contain, because the lookup is a concept check
rather than a runtime search.

**`type`** is an ordinary C++ type. Which types are admissible depends on the
descriptor: `basic_field` takes integrals, the record descriptors take another
`struct_field_list`, and the string and vector descriptors fix the type
themselves.

**`size`** says how many bytes the field occupies on the wire, which is not the
same question as how large its C++ type is. A `u32` field can be declared to
occupy two bytes. The axis has six forms and enough depth to warrant
[its own section](#the-size-axis).

**`constraint`** narrows which values are legal. It is checked in both
directions — while reading, before the value is handed back; while writing,
before the field's first byte is emitted — and a violation is a
`validation_failure` either way. See [Constraints and validation](#constraints-and-validation).

### The descriptors

Each descriptor is an alias that fixes some axes and leaves the rest to you.
`constraint` is the last parameter of every descriptor that accepts one, and
defaults to `no_constraint`.

| Descriptor | Type it holds | Size axis |
|---|---|---|
| `basic_field<id, T, size, c>` | integral `T` | yours, must be `fixed` and fit `sizeof(T)` |
| `fixed_array_field<id, T, N, c>` | `std::array<T, N>` | fixed at `N * sizeof(T)` |
| `c_arr_field<id, T, N, c>` | `T[N]` | fixed at `N * sizeof(T)` |
| `array_of_records<id, T, N, c>` | `std::array<T, N>`, `T` a schema | `size_dont_care` |
| `fixed_string_field<id, N, c>` | `fixed_string<N>` | fixed at `N + 1` |
| `c_str_field<id, N, c>` | `char[N + 1]` | fixed at `N + 1` |
| `str_field<id, size, c>` | `std::string` | yours, must be variable |
| `vec_field<id, T, size, c>` | `std::vector<T>` | yours, must be variable |
| `vector_of_records<id, T, size, c>` | `std::vector<T>`, `T` a schema | yours, must be variable |
| `struct_field<id, T>` | `T`, a schema | `size_dont_care` — no constraint axis |
| `magic_byte_array<id, N, expected>` | `std::array<unsigned char, N>` | fixed at `N` |
| `magic_string<id, expected>` | `fixed_string<expected.size()>` | fixed at `expected.size() + 1` |
| `magic_number<id, T, size, expected>` | integral `T` | yours |
| `maybe<field, present_only_if>` | wraps another descriptor | inherited from it |
| `variance<id, deducer>` | a `std::variant` of the alternatives | from the deduction |

Three things in that table are easy to misread.

The `+ 1` on `fixed_string_field`, `c_str_field` and `magic_string` is a
terminator byte: a `fixed_string<3>` such as `"PKT"` occupies four bytes on the
wire, not three.

`size_dont_care` is not a size of zero. It marks a field whose width is whatever
its own nested schema turns out to occupy, which is why the record descriptors
use it and why you never write it yourself.

The magic descriptors are not a separate mechanism. Each is an ordinary
descriptor with an `eq` constraint already applied, which is why they take an
`expected` value where the others take a constraint.

`maybe` and `variance` are combinators rather than field kinds — they wrap or
select among the descriptors above. They are covered in
[Optional and variant fields](#optional-and-variant-fields).

### The size axis

The size axis answers one question: how many bytes does this field occupy on
the wire? That is not the same question as how large its C++ type is. A field
whose type is `u32` can be declared to occupy two bytes, and reading it will
produce a `u32` holding a value that arrived as two bytes.

Sizes are written as `field_size<...>` wrapping one of six forms.

| Form | How the width is determined |
|---|---|
| `fixed<N>` | exactly `N` bytes, fixed at compile time |
| `len_from_field<"id">` | the value currently held by the field named `id` |
| `size_from_fields<f, with_fields<...>>` | a callable applied to the named sibling fields |
| `len_from_fields<f, ids>` | an alias for `size_from_fields` |
| `size_choices<...>` | one of several widths, chosen per union alternative |
| `size_dont_care` | whatever the field's own nested schema occupies |

**`fixed<N>`** is the only form for a `basic_field`, and it is constrained:
`field_fits_to_underlying_type` requires `N <= sizeof(T)`. Declaring a
`basic_field<"x", u16, field_size<fixed<4>>>` is a compile error, because four
bytes off the wire cannot be delivered into a two-byte type.

**`len_from_field<"id">`** takes the width from another field's value — the
length-prefix pattern. The named field must appear earlier in the schema, since
reading is strictly left to right and the length has to be known before the
field it sizes.

**`size_from_fields<f, with_fields<...>>`** covers everything a single field
lookup cannot express: a length in units other than bytes, a total minus a
header, a width assembled from two fields. The callable and its inputs are
described in [Computed values](#computed-values); `len_from_fields` is the same
template under a name that reads better at a length.

**`size_dont_care`** is what the record descriptors use. It is not zero and it
is not "unknown" — it means the width is whatever the nested schema works out
to, so there is nothing for you to declare. You never write it yourself.

#### Invertible sizes, and why the write path cares

The distinction that matters on the write path is whether a size can be run
backwards.

`len_from_field<"n">` can. If the container holds four elements then `n` is
four, so the library **derives** `n` from the data and makes assigning to it a
compile error — the two can never disagree because only one of them is yours to
set.

`size_from_fields<f, ...>` cannot. `f` is an arbitrary user callable; knowing
its output does not tell you what its inputs were. So its source fields stay
assignable and the library **verifies** instead: it applies `f` at write time
and fails with `found_contradicting_length` if the answer disagrees with the
container's real size.

Reading is unaffected by the distinction — both forms simply produce a width.
[Writing](#writing) covers the consequences in full.

#### Computed values

Three different axes need a value worked out from other fields rather than
stated outright: a length the wire does not carry directly, a predicate deciding
whether an optional field is present, a test choosing a union alternative. They
share one shape — a callable, plus the names of the fields it reads.

**`with_fields<"a", "b">`** is that list of names. It carries no values; it tells
the library which siblings to fetch and in what order, and they are passed to the
callable as arguments in exactly that order.

**`compute<callable, R, Fs>`** is the general form: apply `callable` to the
fields named by `Fs`, producing an `R`. The callable is a non-type template
argument, so it must be usable in a constant expression — a captureless lambda
or a function object declared where the schema can name it.

Everything else in this family is that template under a shorter name.

| Alias | Expands to | Reads as |
|---|---|---|
| `predicate<f, Fs>` | `compute<f, bool, Fs>` | a yes/no test over sibling fields |
| `eval_bool_from_fields<f, Fs>` | `compute<f, bool, Fs>` | the same thing, older spelling |
| `parse_if<f, Fs>` | `eval_bool_from_fields<f, Fs>` | is this optional field present? |
| `eval_size_from_fields<f, Fs>` | `compute<f, std::size_t, Fs>` | a count derived from siblings |

`compute` is documented here as a mechanism you can use directly, not as an
implementation detail behind those aliases. Deriving a `u32` from two sibling
fields is a legitimate thing to write, and the test suite already does it.
**Treat the signature of `compute<callable, R, Fs>` as stable**: documenting the
general form commits the project to keeping it, not just the aliases built on it.

One near relative is deliberately not in that table. `size_from_fields`, and its
alias `len_from_fields`, live on the size axis and take the same
callable-plus-`with_fields` shape, but they are a separate template rather than
an alias of `compute`. The distinction rarely matters when writing a schema and
matters a great deal when reading a compiler error, which is why it is called out
here.

<!-- docs: test/single_header/guide_computed_example.cpp -->
```cpp
#include "s2s.hpp"

#include <sstream>
#include <vector>

using namespace s2s_literals;

using u16 = unsigned short;
using u32 = unsigned int;

// The callable is a non-type template argument, so it has to be usable in a
// constant expression and named somewhere the schema can reach.
constexpr auto area_of = [](auto rows, auto cols) { return rows * cols; };

using matrix =
  s2s::struct_field_list<
    s2s::basic_field<"rows", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"cols", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::vec_field<
      "cells",
      u16,
      s2s::field_size<s2s::len_from_fields<area_of, s2s::with_fields<"rows", "cols">>>
    >
  >;

auto main() -> int {
  matrix obj{};
  // "rows" and "cols" stay assignable: area_of cannot be run backwards, so
  // the library verifies them against cells.size() rather than deriving them.
  obj["rows"_f] = 2u;
  obj["cols"_f] = 3u;
  obj["cells"_f] = std::vector<u16>{1, 2, 3, 4, 5, 6};

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  if(const auto written = s2s::struct_write_be<matrix>(stream, obj); !written)
    return 1;

  const auto back = s2s::struct_cast_be<matrix>(stream);
  if(!back)
    return 1;

  // A disagreement is caught rather than silently written.
  matrix wrong = obj;
  wrong["cols"_f] = 4u;
  std::stringstream discard(std::ios::in | std::ios::out | std::ios::binary);
  const auto rejected = s2s::struct_write_be<matrix>(discard, wrong);

  return (*back)["cells"_f].size() == 6
      && !rejected
      && rejected.error().failure_reason == s2s::error_reason::found_contradicting_length
        ? 0 : 1;
}
```

#### `size_choices` is not currently declarable

`size_choices` exists to give a union alternative its own width, and the
library's internal union machinery uses it. You cannot currently write one into
a schema: `is_selectable_size_v` reads the wrong trait, so the concept that
would admit a `size_choices` rejects it and admits fixed sizes instead. See
`dev/issues/027-is-selectable-size-trait-reads-wrong-trait.md`. It is listed
here for completeness; treat it as unavailable until that is fixed.

### `fixed_string`

`fixed_string<N>` is part of the supported surface, not an internal detail. You
cannot avoid it: `magic_string` and `fixed_string_field` use it as their field
type, so assigning to one means constructing one.

```cpp
obj["magic"_f] = s2s::fixed_string<3>("PKT");
```

`N` is the length in characters, excluding the terminator, so the argument is a
4-character string literal. It offers `data()`, `size()`, `to_sv()`, and
comparison against other `fixed_string`s of any length.

### What this guide does not cover

`include/lib/` also holds `static_vector`, `static_map`, and a set of
metaprogramming helpers. These are implementation machinery for keeping compiler
diagnostics readable, they have no user-facing uses, and they are deliberately
undocumented. Treat them as internal and subject to change; `fixed_string` is
the exception, for the reason above.

## Field Descriptors
```cpp
using our_struct = 
  s2s::struct_field_list<
    s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
    s2s::str_field<"str", s2s::field_size<s2s::len_from_field<"len">>>
  >;
```
The alias list below is the reference form of the table above; it moves into the
Reference section once the guide's ordering settles.

```cpp
template <fixed_string id, integral T, fixed_size_like size_type, auto constraint_on_value = no_constraint<T>{}>
  requires field_fits_to_underlying_type<size_type, T>
using basic_field = field<id, T, size_type, constraint_on_value>;

template <fixed_string id, field_containable T, std::size_t N, auto constraint_on_value = no_constraint<std::array<T, N>>{}>
using fixed_array_field = field<id, std::array<T, N>, field_size<fixed<N * sizeof(T)>>, constraint_on_value>;

template <fixed_string id, field_list_like T, std::size_t N, auto constraint_on_value = no_constraint<std::array<T, N>>{}>
using array_of_records = field<id, std::array<T, N>, field_size<size_dont_care>, constraint_on_value>;

template <fixed_string id, std::size_t N, auto constraint_on_value = no_constraint<fixed_string<N>>{}>
using fixed_string_field = field<id, fixed_string<N>, field_size<fixed<N + 1>>, constraint_on_value>;

template <fixed_string id, field_containable T, std::size_t N, auto constraint_on_value = no_constraint<T[N]>{}>
using c_arr_field = field<id, T[N], field_size<fixed<N * sizeof(T)>>, constraint_on_value>;

template <fixed_string id, std::size_t N, auto constraint_on_value = no_constraint<char[N + 1]>{}>
using c_str_field = field<id, char[N + 1], field_size<fixed<N * sizeof(char) + 1>>, constraint_on_value>;

template <fixed_string id, std::size_t N, auto expected>
using magic_byte_array = field<id, std::array<unsigned char, N>, field_size<fixed<N>>, eq{expected}>;

template <fixed_string id, fixed_string expected>
using magic_string = field<id, fixed_string<expected.size()>, field_size<fixed<expected.size() + 1>>, eq{expected}>;

template <fixed_string id, integral T, fixed_size_like size, auto expected>
using magic_number = field<id, T, size, eq{expected}>;

// todo get vector length in bytes instead of size to read additional overload
// todo how user can provide user defined vector impl or allocator
template <fixed_string id, typename T, variable_size_like size, auto constraint_on_value = no_constraint<std::vector<T>>{}>
using vec_field = field<id, std::vector<T>, size, constraint_on_value>;

template <fixed_string id, field_list_like T, variable_size_like size, auto constraint_on_value = no_constraint<std::vector<T>>{}>
using vector_of_records = field<id, std::vector<T>, size, constraint_on_value>;

// todo check if this will work for all char types like wstring
template <fixed_string id, variable_size_like size, auto constraint_on_value = no_constraint<std::string>{}>
using str_field = field<id, std::string, size, constraint_on_value>;

template <fixed_string id, field_list_like T>
using struct_field = field<id, T, field_size<size_dont_care>, no_constraint<T>{}>;
```


## Cast
```cpp
template <struct_field_list_like T, stream_like S>
auto struct_cast_le(S& stream) -> std::expected<T, cast_error>;

template <struct_field_list_like T, stream_like S>
auto struct_cast_be(S& stream) -> std::expected<T, cast_error>;
```
The APIs struct_cast_xx reads from a stream into struct_field_list, when 
provided a stream as a runtime argument and a struct-schema as a template argument. 
The xx is either le or be denoting byteorder of all the struct members.
The APIs return std::expected which either contains a struct_field_list or read_error


## Writing to stream
```cpp
template <struct_field_list_like T, output_stream_like S>
[[nodiscard]] auto stream_cast_le(S& stream, const T& obj) -> std::expected<void, cast_error>;

template <struct_field_list_like T, output_stream_like S>
[[nodiscard]] auto stream_cast_be(S& stream, const T& obj) -> std::expected<void, cast_error>;
```
The APIs stream_cast_xx serialize a struct_field_list to a stream. They mirror
struct_cast_xx: the same schema drives both directions, the xx suffix picks the
byte order of every member, and failures come back as the same cast_error.
There is nothing to return on success, so the expected holds void.

The struct is written strictly left to right, one field at a time, in
declaration order and at every nesting level.

### A worked example
```cpp
#include "s2s.hpp"
#include <sstream>
#include <vector>

using namespace s2s_literals;
using u16 = unsigned short;
using u32 = unsigned int;

using our_struct =
  s2s::struct_field_list<
    s2s::magic_string<"magic", "S2S">,
    s2s::basic_field<"count", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::vec_field<"data", u16, s2s::field_size<s2s::len_from_field<"count">>>
  >;

auto main() -> int {
  our_struct obj{};
  obj["magic"_f] = s2s::fixed_string<3>("S2S");
  obj["data"_f] = std::vector<u16>{0x1122, 0x3344};
  // Note: "count" is never assigned. It is derived from data.size().

  std::stringstream le(std::ios::in | std::ios::out | std::ios::binary);
  if(auto res = s2s::stream_cast_le<our_struct>(le, obj); !res)
    return 1;

  std::stringstream be(std::ios::in | std::ios::out | std::ios::binary);
  if(auto res = s2s::stream_cast_be<our_struct>(be, obj); !res)
    return 1;

  // Read either one back with the matching byte order.
  auto back = s2s::struct_cast_be<our_struct>(be);
  return back && (*back)["count"_f] == 2 ? 0 : 1;
}
```

### Derived fields are read-only
Some fields are not data you supply — they are consequences of data you
supply. Writing them from the struct would let the two disagree, producing a
stream that does not read back as what you handed in. So the library derives
them, ignores whatever the struct holds, and makes assigning to them a
compile error.

Two kinds of field are derived:

| Field | Derived from |
|---|---|
| the target of a `len_from_field<"n">` size | the container's `size()` |
| the `match_field` discriminant of a `type_switch` variance | the `match_case` value of the alternative currently held |

Both are invertible, which is what makes deriving them possible at all.

```cpp
our_struct obj{};
obj["data"_f] = std::vector<u16>{1, 2, 3};
obj["count"_f] = 3;   // does not compile
```

The non-const `operator[]` returns a const reference for a derived field, so
the diagnostic is a plain assignment-to-read-only error. Reading still works
through either subscript:

```cpp
const auto n = obj["count"_f];                 // compiles
const auto m = std::as_const(obj)["count"_f];  // compiles
```

Note what those reads give you: **the stored slot, not the derived value.**
Derivation happens during the write, against the struct you pass as `const`,
and the result is never written back. On a struct you built yourself the slot
is whatever it was default-constructed to:

```cpp
our_struct obj{};
obj["data"_f] = std::vector<u16>{1, 2, 3};
assert(obj["count"_f] == 0);   // not 3 — nothing has been written yet
```

On a struct that came from `struct_cast` the slot holds what was parsed off
the wire, which is the value you usually want. If you need the length before
writing, ask the container: `obj["data"_f].size()`.

**This is a breaking change to existing read-side code.** If you previously
parsed a struct and then adjusted a length field in place:

```cpp
auto parsed = *s2s::struct_cast_le<our_struct>(stream);
parsed["count"_f] = 5;   // used to compile, no longer does
```

replace it by assigning the container instead. The length follows:

```cpp
parsed["data"_f] = std::vector<u16>(5);   // "count" becomes 5 on the next write
```

Fields that are *not* derived stay assignable, including ones that look
similar: the sources feeding a `len_from_fields<callable, ...>` callable, the
siblings feeding a `parse_if` predicate, the fields feeding an
`if_else_ladder` branch, and a length whose only container sits inside a
`maybe` or a union alternative. None of those can be inverted, so they remain
yours to set — and are verified rather than derived.

### What is checked at write time
Everything the library can check without a second pass is checked *before*
the offending field's first byte is emitted.

| Inconsistency | `failure_reason` | `failed_at` |
|---|---|---|
| `constraint_checker` rejects the value (e.g. a wrong magic value) | `validation_failure` | that field |
| a `parse_if` predicate disagrees with the optional's `has_value()` | `validation_failure` | the optional field |
| an `if_else_ladder` or computed-switch selects an alternative other than the one held | `validation_failure` | the union field |
| no ladder branch and no `match_case` matches at all | `type_deduction_failure` | the union field |
| a derived or verified length does not fit its declared width | `validation_failure` | the length field |
| a `len_from_fields` callable disagrees with the container's real size | `found_contradicting_length` | the container field |
| two containers sharing one length field imply different lengths | `found_contradicting_length` | the length field |
| a length inside a `maybe` or union alternative disagrees while that producer is active | `found_contradicting_length` | the length field |
| the underlying stream refuses the write | `buffer_exhaustion` | the field being written |

For a violation inside a nested record, `failed_at` names the outermost record
field rather than the inner one, matching how the read path reports nested
failures.

### Write-once, fail-fast
There is no rollback and no size query. Bytes handed to your stream are gone,
so the unit of atomicity is one field rather than the whole write:

- every derivation, verification and constraint check for a field runs before
  its first byte is emitted;
- a failure attributed to field `k` therefore leaves fields `0..k-1` in the
  stream and nothing of `k` — a well-defined prefix.

The one exception is a stream that fails part way through a single field: how
much it consumed is the stream's business, and the library reports
`buffer_exhaustion`. Recovering from a partial write is the caller's
responsibility — truncate, discard, or write to a staging buffer first if you
need all-or-nothing.


## Errors
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
A read can fail in three ways: field value validation failure, the input
stream is exhausted, or type deduction failed while reading into a union.

A write can fail in those same three ways plus `found_contradicting_length`,
which means two parts of the struct imply different lengths for the same data
— a cross-field disagreement rather than a value that is wrong on its own
terms. See the table above for which check produces which reason.
