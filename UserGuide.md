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

### Optional and variant fields

Two combinators sit on top of the descriptors. `maybe` makes a field's
*presence* conditional; `variance` makes its *type* conditional. Both decide by
looking at fields already parsed, using the computed-value machinery above.

#### `maybe` — presence deduction

```cpp
s2s::maybe<
  s2s::basic_field<"detail", u32, s2s::field_size<s2s::fixed<4>>>,
  s2s::parse_if<kind_is_one, s2s::with_fields<"kind">>
>
```

The first argument is an ordinary descriptor; the second is a `parse_if` whose
predicate decides whether the field is on the wire at all. Reading, a false
predicate means the field is skipped and the stream position does not move.
Writing, the predicate is checked against whether the struct actually holds a
value, and a disagreement is a `validation_failure` — the predicate is not
allowed to say "present" while the struct is empty, because the result would not
read back.

The field's value is reached by dereferencing:

```cpp
*(obj["detail"_f])
```

<!-- docs: test/single_header/guide_optional_example.cpp -->
```cpp
#include "s2s.hpp"

#include <sstream>

using namespace s2s_literals;

using u32 = unsigned int;

// "detail" is on the wire only when "kind" is 1. The predicate reads siblings
// that have already been parsed, which is why it must name them.
constexpr auto kind_is_one = [](auto kind) { return kind == 1u; };

using message =
  s2s::struct_field_list<
    s2s::basic_field<"kind", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::maybe<
      s2s::basic_field<"detail", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::parse_if<kind_is_one, s2s::with_fields<"kind">>
    >
  >;

auto write_and_read(u32 kind, bool with_detail) -> bool {
  message obj{};
  obj["kind"_f] = kind;
  if(with_detail)
    obj["detail"_f] = 0xbeefbeefu;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  const auto written = s2s::struct_write_be<message>(stream, obj);
  if(!written)
    return false;

  const auto back = s2s::struct_cast_be<message>(stream);
  if(!back)
    return false;

  // An absent optional is empty; a present one is dereferenced.
  return with_detail ? *((*back)["detail"_f]) == 0xbeefbeefu : true;
}

auto main() -> int {
  // kind == 1: the predicate says present, and the struct supplies it.
  if(!write_and_read(1u, true))
    return 1;

  // kind == 2: the predicate says absent, and the struct leaves it unset.
  if(!write_and_read(2u, false))
    return 1;

  // Disagreement is caught: the predicate says present, the struct is empty.
  message bad{};
  bad["kind"_f] = 1u;
  std::stringstream discard(std::ios::in | std::ios::out | std::ios::binary);
  const auto rejected = s2s::struct_write_be<message>(discard, bad);

  return !rejected
      && rejected.error().failure_reason == s2s::error_reason::validation_failure
        ? 0 : 1;
}
```

#### `variance` — type deduction

`variance<id, type<...>>` produces a field whose C++ type is a `std::variant` of
the alternatives, reached with `std::get`. There are two ways to say which
alternative applies, and they differ in a way that matters on the write path.

**By discriminant.** `type<match_field<"tag">, type_switch<match_case<v, tag>...>>`
reads the field named by `match_field` and picks the `match_case` whose value
matches. The discriminant can also be computed rather than read directly, by
putting a `compute<...>` where the `match_field` goes.

**By predicate.** `type<type_if_else<branch<predicate, tag>...>>` evaluates each
branch's predicate in order and takes the first that holds. There is no
discriminant field and no `match_field` — nothing about the choice is on the
wire.

> The directory `include/type_deduction/if_else_ladder/` gives this construct its
> informal name, but `if_else_ladder` is not an identifier. The types are
> `type_if_else` and `branch`.

The alternatives are named with type tags rather than descriptors, because an
alternative has no id of its own — it inherits the `variance`'s.

| Tag | Alternative type |
|---|---|
| `as_trivial<T, size>` | integral `T` |
| `as_struct<T>` | a nested schema |
| `as_fixed_arr<T, N>` | `std::array<T, N>` |
| `as_arr_of_records<T, N>` | `std::array<T, N>` of schemas |
| `as_vec<T, size>` | `std::vector<T>` |
| `as_vec_of_records<T, size>` | `std::vector<T>` of schemas |
| `as_string<size>` | `std::string` |
| `as_fixed_string<N>` | `fixed_string<N>` |

**Which form you choose decides whether the discriminant is yours to set.** A
`match_case` value can be run backwards — if the variant holds the alternative
tagged `2`, the discriminant is `2` — so the library derives the `match_field`
target and makes assigning to it a compile error. A `branch` predicate cannot be
inverted, so the fields feeding it stay assignable and are verified instead: at
write time the predicates are re-evaluated, and if the branch they select is not
the alternative being held, the write fails with `validation_failure`.

If no branch holds and no `match_case` matches, the failure is
`type_deduction_failure` rather than `validation_failure` — the library could not
work out which alternative applies at all, as opposed to finding one that
disagrees.

<!-- docs: test/single_header/guide_variance_example.cpp -->
```cpp
#include "s2s.hpp"

#include <sstream>
#include <variant>

using namespace s2s_literals;

using u32 = unsigned int;
using i32 = int;

// Selection by discriminant: "tag" says which alternative "body" holds.
using switched =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<1, s2s::as_trivial<u32, s2s::field_size<s2s::fixed<4>>>>,
          s2s::match_case<2, s2s::as_trivial<i32, s2s::field_size<s2s::fixed<4>>>>
        >
      >
    >
  >;

// Selection by predicate: no discriminant on the wire at all, so there is no
// match_field and nothing for the library to derive.
constexpr auto sum_is_small = [](auto a, auto b) { return a + b < 100u; };
constexpr auto sum_is_large = [](auto a, auto b) { return a + b >= 100u; };

using laddered =
  s2s::struct_field_list<
    s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::type_if_else<
          s2s::branch<
            s2s::predicate<sum_is_small, s2s::with_fields<"a", "b">>,
            s2s::as_trivial<u32, s2s::field_size<s2s::fixed<4>>>
          >,
          s2s::branch<
            s2s::predicate<sum_is_large, s2s::with_fields<"a", "b">>,
            s2s::as_trivial<i32, s2s::field_size<s2s::fixed<4>>>
          >
        >
      >
    >
  >;

auto main() -> int {
  // "tag" is never assigned: it is derived from which alternative body holds.
  switched s{};
  s["body"_f] = i32{-7};

  std::stringstream a(std::ios::in | std::ios::out | std::ios::binary);
  if(const auto written = s2s::struct_write_be<switched>(a, s); !written)
    return 1;
  const auto s_back = s2s::struct_cast_be<switched>(a);
  if(!s_back || (*s_back)["tag"_f] != 2 || std::get<i32>((*s_back)["body"_f]) != -7)
    return 1;

  // "a" and "b" stay assignable: a predicate has no inverse, so they are
  // verified against the alternative held rather than derived from it.
  laddered l{};
  l["a"_f] = 10u;
  l["b"_f] = 20u;
  l["body"_f] = u32{42};

  std::stringstream b(std::ios::in | std::ios::out | std::ios::binary);
  if(const auto written = s2s::struct_write_be<laddered>(b, l); !written)
    return 1;
  const auto l_back = s2s::struct_cast_be<laddered>(b);
  if(!l_back || std::get<u32>((*l_back)["body"_f]) != 42)
    return 1;

  // The held alternative contradicts the branch the predicates select.
  laddered wrong{};
  wrong["a"_f] = 500u;
  wrong["b"_f] = 500u;
  wrong["body"_f] = u32{42};
  std::stringstream discard(std::ios::in | std::ios::out | std::ios::binary);
  const auto rejected = s2s::struct_write_be<laddered>(discard, wrong);

  return !rejected
      && rejected.error().failure_reason == s2s::error_reason::validation_failure
        ? 0 : 1;
}
```

Two constraints are enforced on a `variance` at compile time: the alternatives
must be distinct types, and the `match_case` values must be unique.

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

## Constraints and validation

The `constraint` axis narrows which values a field may hold. It is the last
parameter of every descriptor that takes one, and it defaults to
`no_constraint`, so leaving it off means "any value the type can represent".

A constraint is checked in **both** directions:

- **reading** — after the bytes are decoded, before the value is stored;
- **writing** — before the field's first byte reaches the stream.

A violation is `validation_failure` either way, with `failed_at` naming the
field. This symmetry is the point: a schema that refuses to parse a bad value
also refuses to produce one.

### The constructs

| Constraint | Holds when | Value type |
|---|---|---|
| `no_constraint<T>{}` | always | any |
| `eq{v}` | `value == v` | equality-comparable, non-floating-point |
| `neq{v}` | `value != v` | equality-comparable, non-floating-point |
| `lt{v}` | `value < v` | comparable |
| `gt{v}` | `value > v` | comparable |
| `lte{v}` | `value <= v` | comparable |
| `gte{v}` | `value >= v` | integral or `fixed_string` |
| `any_of{a, b, ...}` | `value` is one of the listed values | all arguments the same type |

They are used as values, not types, and CTAD deduces the parameter:

```cpp
s2s::basic_field<"version", u16, s2s::field_size<s2s::fixed<2>>, s2s::any_of{u16{1}, u16{2}}>
```

Two type restrictions are worth knowing before a template error explains them.
`eq` and `neq` exclude floating-point types outright — equality on a float
decoded from a wire format is a trap, so the library refuses it rather than
supporting it badly. `gte` is narrower still, admitting only integrals and
`fixed_string`.

### Range constraints do not currently compile

`range`, `is_in_open_range` and `is_in_closed_range` are present in the headers
and are **not usable**. `range`'s constructor applies `static_assert` to its own
function parameters, which are not constant expressions, so constructing one is
a hard compile error — and both range constraints take `range` arguments.

```cpp
constexpr s2s::range<int> r{1, 5};   // error: 'value1' is not a constant expression
```

See `dev/issues/026-range-constraints-do-not-compile.md`. Until that is fixed,
express a bounded field with `gte`/`lte` on adjacent fields or with `any_of` for
a small set. This section will list them among the working constructs once they
work.

### The magic descriptors are constraints

`magic_string`, `magic_number` and `magic_byte_array` are not a separate
validation mechanism. Each is an ordinary descriptor whose constraint axis is
already filled in with `eq`:

```cpp
magic_string<"magic", "S2S">
// is
field<"magic", fixed_string<3>, field_size<fixed<4>>, eq{fixed_string<3>("S2S")}>
```

which is why they take an `expected` value where other descriptors take a
constraint, and why a wrong magic value reports `validation_failure` like any
other constraint breach rather than an error of its own.

<!-- docs: test/single_header/guide_constraints_example.cpp -->
```cpp
#include "s2s.hpp"

#include <sstream>

using namespace s2s_literals;

using u16 = unsigned short;
using u32 = unsigned int;

using record =
  s2s::struct_field_list<
    s2s::magic_string<"magic", "S2S">,
    s2s::basic_field<"version", u16, s2s::field_size<s2s::fixed<2>>, s2s::any_of{u16{1}, u16{2}}>,
    s2s::basic_field<"count", u32, s2s::field_size<s2s::fixed<4>>, s2s::gte{u32{1}}>
  >;

auto main() -> int {
  record ok{};
  ok["magic"_f] = s2s::fixed_string<3>("S2S");
  ok["version"_f] = u16{2};
  ok["count"_f] = 7u;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  if(const auto written = s2s::struct_write_be<record>(stream, ok); !written)
    return 1;
  if(!s2s::struct_cast_be<record>(stream))
    return 1;

  // Writing: the constraint is checked before the field's first byte is out.
  record bad_version = ok;
  bad_version["version"_f] = u16{9};
  std::stringstream discard(std::ios::in | std::ios::out | std::ios::binary);
  const auto rejected = s2s::struct_write_be<record>(discard, bad_version);
  if(!(!rejected
       && rejected.error().failure_reason == s2s::error_reason::validation_failure
       && rejected.error().failed_at == std::string_view{"version"}))
    return 1;

  // Reading: the same constraint rejects the same value off the wire.
  record bad_count = ok;
  bad_count["count"_f] = 0u;
  std::stringstream on_wire(std::ios::in | std::ios::out | std::ios::binary);
  // Write it through a schema with no constraint so the bytes actually exist.
  using unchecked =
    s2s::struct_field_list<
      s2s::magic_string<"magic", "S2S">,
      s2s::basic_field<"version", u16, s2s::field_size<s2s::fixed<2>>>,
      s2s::basic_field<"count", u32, s2s::field_size<s2s::fixed<4>>>
    >;
  unchecked loose{};
  loose["magic"_f] = s2s::fixed_string<3>("S2S");
  loose["version"_f] = u16{2};
  loose["count"_f] = 0u;
  if(const auto written = s2s::struct_write_be<unchecked>(on_wire, loose); !written)
    return 1;

  const auto read_back = s2s::struct_cast_be<record>(on_wire);
  return !read_back
      && read_back.error().failure_reason == s2s::error_reason::validation_failure
      && read_back.error().failed_at == std::string_view{"count"}
        ? 0 : 1;
}
```

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


## Reading

```cpp
template <struct_field_list_like T, stream_like S>
auto struct_cast_le(S& stream) -> std::expected<T, cast_error>;

template <struct_field_list_like T, stream_like S>
auto struct_cast_be(S& stream) -> std::expected<T, cast_error>;
```

The schema is the template argument, the stream is the runtime one. The `le` and
`be` suffix fixes the byte order of every integral member of the schema — it is
not a per-field setting. Success gives you the populated `struct_field_list`;
failure gives you a `cast_error`.

The struct is read strictly left to right, one field at a time, in declaration
order and at every nesting level. That ordering is not an implementation detail
you can ignore: it is why a `len_from_field` must name a field declared *before*
the one it sizes, and why a `parse_if` predicate can only read fields already
parsed.

### A worked example

<!-- docs: test/single_header/guide_reading_example.cpp -->
```cpp
#include "s2s.hpp"

#include <sstream>
#include <string>

using namespace s2s_literals;

using u16 = unsigned short;
using u32 = unsigned int;

using header =
  s2s::struct_field_list<
    s2s::magic_string<"magic", "S2S">,
    s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::str_field<"name", s2s::field_size<s2s::len_from_field<"len">>>,
    s2s::basic_field<"flags", u16, s2s::field_size<s2s::fixed<2>>>
  >;

// The bytes a big-endian "header" occupies, written out by hand:
//   53 32 53 00     magic, "S2S" plus its terminator
//   00 00 00 05     len = 5
//   68 65 6c 6c 6f  name, "hello", exactly len bytes
//   00 01           flags = 1
constexpr auto on_the_wire =
  "\x53\x32\x53\x00"
  "\x00\x00\x00\x05"
  "hello"
  "\x00\x01";

auto main() -> int {
  std::stringstream stream(std::string(on_the_wire, 15),
                           std::ios::in | std::ios::out | std::ios::binary);

  const auto parsed = s2s::struct_cast_be<header>(stream);
  if(!parsed)
    return 1;

  const auto& h = *parsed;
  if(!(h["len"_f] == 5 && h["name"_f] == "hello" && h["flags"_f] == 1))
    return 1;

  // A stream that stops short reports where it ran out, not merely that it did.
  std::stringstream truncated(std::string(on_the_wire, 9),
                              std::ios::in | std::ios::out | std::ios::binary);
  const auto failed = s2s::struct_cast_be<header>(truncated);

  return !failed
      && failed.error().failure_reason == s2s::error_reason::buffer_exhaustion
      && failed.error().failed_at == std::string_view{"name"}
        ? 0 : 1;
}
```

### The field kinds

Every kind of field the library reads is a descriptor from
[Schema](#schema); nothing here is read-only vocabulary. What differs per kind
is how many bytes get consumed and where that count comes from.

| What is on the wire | Declared as | Bytes consumed |
|---|---|---|
| Trivial | `basic_field` | its `fixed<N>` |
| Array of trivials | `fixed_array_field`, `c_arr_field` | `N * sizeof(T)`, fixed |
| Array of records | `array_of_records` | sum of `N` nested schemas |
| Length-prefixed vector of trivials | `vec_field` | element count from the size axis |
| Length-prefixed vector of records | `vector_of_records` | element count from the size axis |
| Const-sized string | `fixed_string_field`, `c_str_field` | `N + 1`, terminator included |
| Length-prefixed string | `str_field` | byte count from the size axis |
| Optional | `maybe` | zero, or the wrapped field's |
| Union | `variance` | the selected alternative's |
| Magic string | `magic_string` | `size() + 1`, and the value is checked |
| Magic number | `magic_number` | its declared size, and the value is checked |
| Nested record | `struct_field` | whatever the nested schema occupies |

Two of these consume a variable number of bytes on a *per-read* basis rather
than a per-schema one: `maybe` consumes nothing when its predicate is false, and
`variance` consumes whatever the selected alternative needs. Everything else has
a width that is either fixed at compile time or read from an earlier field.

### What a failed read leaves behind

`struct_cast` returns `std::expected`, so a failure gives you a `cast_error` and
no struct — there is no partially populated result to inspect. What the library
had parsed up to that point is discarded along with the `expected`.

The stream is a different matter. It is yours, the library does not own it, and
on failure its read position is wherever the failing field stopped — not rewound
to where the cast started. If you intend to retry, or to try a second schema
against the same bytes, record the position yourself before the call. Reading
from a fresh stream over the same buffer is usually simpler.

### Errors

| Cause | `failure_reason` | `failed_at` |
|---|---|---|
| the stream ran out mid-field | `buffer_exhaustion` | the field being read |
| a `constraint_checker` rejected the decoded value | `validation_failure` | that field |
| a magic value did not match | `validation_failure` | the magic field |
| no `match_case` matched and no `branch` predicate held | `type_deduction_failure` | the union field |

`found_contradicting_length`, the fourth `error_reason`, cannot arise on a read.
It reports two parts of a struct implying different lengths for the same data,
which is only detectable when writing — reading takes the length off the wire
and has nothing to disagree with it. See [Writing](#writing).

For a failure inside a nested record, `failed_at` names the outermost record
field rather than the inner one. A validation failure two levels down inside
`struct_field<"header", ...>` reports `"header"`, so the name you get back is
always one that appears in the schema you handed to `struct_cast`.


## Writing

```cpp
template <struct_field_list_like T, output_stream_like S>
[[nodiscard]] auto stream_cast_le(S& stream, const T& obj) -> std::expected<void, cast_error>;

template <struct_field_list_like T, output_stream_like S>
[[nodiscard]] auto stream_cast_be(S& stream, const T& obj) -> std::expected<void, cast_error>;
```

The APIs mirror `struct_cast_xx`: the same schema drives both directions, the
suffix picks the byte order of every member, and failures come back as the same
`cast_error`. There is nothing to return on success, so the `expected` holds
`void`.

The struct is written strictly left to right, one field at a time, in
declaration order and at every nesting level — the same traversal
[Reading](#reading) describes.

What is specific to this direction is that the schema now has to *produce* the
values it previously only consumed. Every construct below is declared in
[Schema](#schema); this section covers only what writing does with them.

### A worked example

<!-- docs: test/single_header/guide_writing_example.cpp -->
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
siblings feeding a `parse_if` predicate, the fields feeding a `branch`
predicate inside a `type_if_else`, and a length whose only container sits
inside a `maybe` or a union alternative. None of those can be inverted, so they
remain yours to set — and are verified rather than derived.

The rule behind that list is [invertibility](#invertible-sizes-and-why-the-write-path-cares):
a construct the library can run backwards is derived and made read-only, and one
it cannot is left to you and checked.

### What is checked at write time
Everything the library can check without a second pass is checked *before*
the offending field's first byte is emitted.

| Inconsistency | `failure_reason` | `failed_at` |
|---|---|---|
| `constraint_checker` rejects the value (e.g. a wrong magic value) | `validation_failure` | that field |
| a `parse_if` predicate disagrees with the optional's `has_value()` | `validation_failure` | the optional field |
| a `type_if_else` or computed-switch selects an alternative other than the one held | `validation_failure` | the union field |
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


## Streams

The library never owns a stream and never creates one. It calls two operations
on whatever you hand it and checks whether the thing still looks healthy
afterwards. That is the whole contract, and satisfying it is how "pluggable
interfaces working with custom streams" is meant to be acted on.

### What a stream must provide

Two concepts gate the public API, and each is a disjunction underneath.

```cpp
template <typename T>
concept input_stream_like =
  (identified_as_constexpr_stream<T> || readable<T>) && convertible_to_bool<T>;

template <typename T>
concept output_stream_like =
  (identified_as_constexpr_stream<T> || writeable<T>) && convertible_to_bool<T>;
```

| Concept | Required expression | Returning |
|---|---|---|
| `read_trait<T>` | `obj.read(char*, std::streamsize)` | `T&` |
| `std_read_trait<T>` | `obj.read(char*, std::streamsize)` | convertible to `std::istream&` |
| `write_trait<T>` | `obj.write(const char*, std::size_t)` | `T&` |
| `std_write_trait<T>` | `obj.write(const char*, std::size_t)` | convertible to `std::ostream&` |
| `readable<T>` | either read form | — |
| `writeable<T>` | either write form | — |
| `convertible_to_bool<T>` | `obj.operator bool()` and `!obj` | `bool` |

The `std_` variants exist so the standard streams work untouched: `std::ifstream`
and `std::stringstream` return `std::istream&` from `read`, which is why they
satisfy `std_read_trait` rather than `read_trait`. Your own type will normally
return `T&` and satisfy the plain form. Either is accepted.

`convertible_to_bool` is not optional and is easy to miss. The library tests the
stream after every access, so it needs both `operator bool` and `operator!` to
yield `bool`. An `explicit operator bool` satisfies both, because `!obj` is a
contextual conversion.

**Failure is reported by going bad, not by throwing.** A stream that cannot
satisfy a read marks itself unhealthy and returns; the library observes that on
the next `bool` test and reports `buffer_exhaustion` against the field it was
working on. Throwing out of `read` or `write` propagates through the library,
which is not designed to be exception-safe part way through a field.

### A reference implementation

<!-- docs: test/single_header/guide_stream_example.cpp -->
```cpp
#include "s2s.hpp"

#include <algorithm>
#include <cstddef>
#include <ios>
#include <vector>

using namespace s2s_literals;

using u32 = unsigned int;

// A complete stream, satisfying read_trait, write_trait and convertible_to_bool.
// Nothing else is required: no seeking, no exceptions, no std::ios base.
class byte_stream {
  std::vector<char> buffer;
  std::size_t read_pos{0};
  bool ok{true};

public:
  // read_trait: takes a raw destination and a count, returns *this by reference.
  auto read(char* dest, std::streamsize size_to_read) -> byte_stream& {
    const auto n = static_cast<std::size_t>(size_to_read);
    if(read_pos + n > buffer.size()) {
      ok = false;               // report exhaustion by going bad, not by throwing
      return *this;
    }
    std::copy_n(buffer.begin() + static_cast<std::ptrdiff_t>(read_pos), n, dest);
    read_pos += n;
    return *this;
  }

  // write_trait: takes a raw source and a count, returns *this by reference.
  auto write(const char* src, std::size_t size_to_write) -> byte_stream& {
    buffer.insert(buffer.end(), src, src + size_to_write);
    return *this;
  }

  // convertible_to_bool: the library tests the stream after every access.
  explicit operator bool() const { return ok; }
};

using our_struct =
  s2s::struct_field_list<
    s2s::magic_string<"magic", "S2S">,
    s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>
  >;

static_assert(s2s::input_stream_like<byte_stream>);
static_assert(s2s::output_stream_like<byte_stream>);

auto main() -> int {
  our_struct obj{};
  obj["magic"_f] = s2s::fixed_string<3>("S2S");
  obj["a"_f] = 0xdeadbeefu;

  byte_stream stream;
  if(const auto written = s2s::struct_write_be<our_struct>(stream, obj); !written)
    return 1;

  const auto back = s2s::struct_cast_be<our_struct>(stream);
  if(!back || (*back)["a"_f] != 0xdeadbeefu)
    return 1;

  // Reading past the end sets the bad flag, which surfaces as buffer_exhaustion.
  const auto overrun = s2s::struct_cast_be<our_struct>(stream);
  return !overrun
      && overrun.error().failure_reason == s2s::error_reason::buffer_exhaustion
        ? 0 : 1;
}
```

The `static_assert`s are worth keeping in your own code. Failing the concept at
the point of definition produces a readable error; failing it at the call to
`struct_cast` produces a template instantiation trace.

### Constexpr streams

A stream usable during constant evaluation is identified by inheritance, not by
its operations:

```cpp
template <std::size_t N>
class memstream: s2s::constexpr_stream { /* ... */ };
```

`s2s::constexpr_stream` is an empty base struct used as a tag, and
`identified_as_constexpr_stream<T>` tests for it. Deriving from it is what puts
a type on the constexpr path — `input_stream_like` and `output_stream_like`
accept such a type without checking its read and write operations at all.

A constexpr stream works on `std::array<char, N>` rather than `char*`, because
pointer arithmetic over a buffer is not available during constant evaluation:

```cpp
template <std::size_t dest_buffer_size>
constexpr auto read(std::array<char, dest_buffer_size>& dest, std::size_t n) -> memstream&;

template <std::size_t src_buffer_size>
constexpr auto write(const std::array<char, src_buffer_size>& src, std::size_t n) -> memstream&;
```

`test/utils/constexpr_memstream.hpp` is a complete worked implementation,
including the `rewind()` needed to read back what was just written.

> Two concepts, `constexpr_read_trait<T, N>` and `constexpr_write_trait<T, N>`,
> describe exactly those two signatures — but nothing in `include/` refers to
> them. A constexpr stream is admitted on the strength of its base class alone,
> so these two currently document the expected shape without enforcing it.
> Deriving from `constexpr_stream` while getting the signatures wrong fails
> later, inside the read or write, rather than at the concept check.

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
