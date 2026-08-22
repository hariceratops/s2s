# Optional and variant fields

Two combinators sit on top of the descriptors. `maybe` makes a field's
*presence* conditional; `variance` makes its *type* conditional. Both decide by
looking at fields already parsed, using the machinery in
[Computed values](computed-values.md).

## `maybe` — presence deduction

```cpp
s2s::maybe<
  s2s::basic_field<"name_length", u16, 2_B>,
  s2s::parse_if<has_name, "flags">
>
```

The first argument is an ordinary descriptor; the second is a `parse_if` whose
predicate decides whether the field is on the wire at all. Reading, a false
predicate means the field is skipped and the stream position does not move.
Writing, the predicate is checked against whether the struct actually holds a
value, and a disagreement is a `validation_failure` — a predicate is not allowed
to claim "present" while the struct is empty, because the result would not read
back.

The value is reached by dereferencing:

```cpp
*(header["name_length"_f])
```

<!-- docs: test/doc_examples/guide_optional_example.cpp -->
```cpp
#include "s2s.hpp"

#include <array>
#include <fstream>

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

// Bit 3 of the flags byte says an original-filename length follows. The
// predicate reads a field already parsed, which is why it must name it.
constexpr auto has_name = [](auto flags) { return (flags & 0x08u) != 0u; };

// A gzip member header, trimmed to the part that shows conditional presence.
using gzip_header =
  s2s::struct_field_list<
    s2s::magic_byte_array<"magic", 2, std::array<u8, 2>{0x1f, 0x8b}>,
    s2s::basic_field<"method", u8, 1_B>,
    s2s::basic_field<"flags", u8, 1_B>,
    s2s::basic_field<"mtime", u32, 4_B>,
    s2s::maybe<
      s2s::basic_field<"name_length", u16, 2_B>,
      s2s::parse_if<has_name, "flags">
    >
  >;

auto round_trip(u8 flags, bool with_name, const char* path) -> bool {
  gzip_header header{};
  header["magic"_f] = std::array<u8, 2>{0x1f, 0x8b};
  header["method"_f] = u8{8};
  header["flags"_f] = flags;
  header["mtime"_f] = 0x5f000000u;
  if(with_name)
    header["name_length"_f] = u16{12};

  std::fstream file(path, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

  return s2s::stream_cast_be<gzip_header>(file, header)
    .and_then([&file] {
      file.seekg(0);
      return s2s::struct_cast_be<gzip_header>(file);
    })
    .transform([with_name](const gzip_header& parsed) {
      // An absent optional is empty; a present one is dereferenced.
      return with_name ? *(parsed["name_length"_f]) == 12 : true;
    })
    .value_or(false);
}

auto main() -> int {
  // FNAME set: the predicate says present, and the header supplies it.
  if(!round_trip(u8{0x08}, true, "gzip_named.bin"))
    return 1;

  // FNAME clear: the predicate says absent, and the field occupies no bytes.
  if(!round_trip(u8{0x00}, false, "gzip_plain.bin"))
    return 1;

  // Disagreement is caught: the flag promises a name length, the header has none.
  gzip_header bad{};
  bad["magic"_f] = std::array<u8, 2>{0x1f, 0x8b};
  bad["flags"_f] = u8{0x08};
  std::fstream discard("gzip_bad.bin",
                       std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

  const auto rejected = s2s::stream_cast_be<gzip_header>(discard, bad);
  if(rejected.has_value())
    return 1;

  return rejected.error().failure_reason == s2s::error_reason::validation_failure ? 0 : 1;
}
```

## `variance` — type deduction

`variance<id, type<...>>` produces a field whose C++ type is a `std::variant` of
the alternatives, reached with `std::get`. There are two ways to say which
alternative applies, and they differ in a way that matters on the write path.

**By discriminant.** `type<match_field<"tag">, type_switch<match_case<v, tag>...>>`
reads the field named by `match_field` and picks the `match_case` whose value
matches. The discriminant can also be computed rather than read directly, by
putting a `compute<...>` where the `match_field` goes.

**By predicate.** `type<type_if_else<branch<predicate, tag>...>>` evaluates each
branch's predicate in order and takes the first that holds. There is no
discriminant field and nothing about the choice is on the wire.

!!! note "`if_else_ladder` is not an identifier"

    The directory `include/type_deduction/if_else_ladder/` gives this construct
    its informal name, but no such type exists. The types are `type_if_else`
    and `branch`.

Alternatives are named with type tags rather than descriptors, because an
alternative has no id of its own — it inherits the `variance`'s.

| Tag | Alternative type |
|---|---|
| `as_trivial<T, opts...>` | integral `T` |
| `as_struct<T, opts...>` | a nested schema |
| `as_fixed_arr<T, N, opts...>` | `std::array<T, N>` |
| `as_arr_of_records<T, N, opts...>` | `std::array<T, N>` of schemas |
| `as_vec<T, opts...>` | `std::vector<T>` |
| `as_vec_of_records<T, opts...>` | `std::vector<T>` of schemas |
| `as_string<opts...>` | `std::string` |
| `as_fixed_string<N, opts...>` | `fixed_string<N>` |

A tag's trailing `opts...` is the same order-independent option pack its mirror
descriptor takes — a size, a constraint and an allocation bound. `as_vec<u8,
len_from_field<"n">>` reads the same as it always did; what changed is that the
size is a pack entry rather than a fixed second argument, which is what lets
further options join it without a new spelling.

Which entries a tag admits follows from the tag. The four whose size is fixed by
the tag itself — `as_struct`, `as_fixed_arr`, `as_arr_of_records` and
`as_fixed_string` — take a constraint and nothing else. The three whose extent
comes off the wire — `as_vec`, `as_string` and `as_vec_of_records` — take a
bound as well. An entry a tag does not admit is a compile error, not a silently
ignored one.

`N` stays positional on `as_fixed_arr`, `as_fixed_string` and
`as_arr_of_records`: it is an element count, not a size value.

`as_trivial<T>` with no size entry defaults to `sizeof(T)`, exactly as
`basic_field<"x", T>` does.

!!! warning "`as_vec_of_records` cannot be written"

    A `as_vec_of_records` alternative reads correctly but fails to write, with
    `found_contradicting_length` at its length field. The length is never
    derived from the held vector, and because it is a length target it cannot be
    assigned either, so there is no way to write one today. Reading is
    unaffected. The other seven tags round-trip.

**Which form is chosen decides whether the discriminant is data anyone
supplies.** A `match_case` value can be run backwards — if the variant holds the
alternative tagged `2`, the discriminant is `2` — so the library derives the
`match_field` target and makes assigning to it a compile error. A `branch`
predicate cannot be inverted, so the fields feeding it stay assignable and are
verified instead: at write time the predicates are re-evaluated, and if the
branch they select is not the alternative being held, the write fails with
`validation_failure`.

If no branch holds and no `match_case` matches, the failure is
`type_deduction_failure` rather than `validation_failure` — the library could
not work out which alternative applies at all, as opposed to finding one that
disagrees.

<!-- docs: test/doc_examples/guide_variance_example.cpp -->
```cpp
#include "s2s.hpp"

#include <fstream>
#include <variant>

using namespace s2s_literals;

using u8 = unsigned char;
using u32 = unsigned int;
using i32 = int;

// Selection by discriminant. A tag-length-value record carries the tag on the
// wire, and it says how the value should be read.
using tlv_record =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u8, 1_B>,
    s2s::variance<
      "value",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<1, s2s::as_trivial<u32, 4_B>>,
          s2s::match_case<2, s2s::as_trivial<i32, 4_B>>
        >
      >
    >
  >;

// Selection by predicate. Nothing on the wire names the payload shape; it
// follows from a length already read, the way formats inline small values and
// reference large ones.
constexpr auto fits_inline = [](auto length) { return length <= 4u; };
constexpr auto needs_reference = [](auto length) { return length > 4u; };

using extent_record =
  s2s::struct_field_list<
    s2s::basic_field<"length", u32, 4_B>,
    s2s::variance<
      "payload",
      s2s::type<
        s2s::type_if_else<
          s2s::branch<
            s2s::predicate<fits_inline, "length">,
            s2s::as_trivial<u32, 4_B>
          >,
          s2s::branch<
            s2s::predicate<needs_reference, "length">,
            s2s::as_trivial<i32, 4_B>
          >
        >
      >
    >
  >;

auto main() -> int {
  // "tag" is never assigned: it is derived from the alternative held.
  tlv_record record{};
  record["value"_f] = i32{-40};

  std::fstream tlv("tlv_record.bin",
                   std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

  const auto tlv_ok =
    s2s::stream_cast_be<tlv_record>(tlv, record)
      .and_then([&tlv] {
        tlv.seekg(0);
        return s2s::struct_cast_be<tlv_record>(tlv);
      })
      .transform([](const tlv_record& parsed) {
        return parsed["tag"_f] == 2 && std::get<i32>(parsed["value"_f]) == -40;
      });

  if(!tlv_ok.value_or(false))
    return 1;

  // "length" stays assignable: a predicate has no inverse, so it is verified
  // against the alternative held rather than derived from it.
  extent_record extent{};
  extent["length"_f] = 4u;
  extent["payload"_f] = u32{0xfeedface};

  std::fstream ext("extent_record.bin",
                   std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

  const auto extent_ok =
    s2s::stream_cast_be<extent_record>(ext, extent)
      .and_then([&ext] {
        ext.seekg(0);
        return s2s::struct_cast_be<extent_record>(ext);
      })
      .transform([](const extent_record& parsed) {
        return std::get<u32>(parsed["payload"_f]) == 0xfeedface;
      });

  if(!extent_ok.value_or(false))
    return 1;

  // The held alternative contradicts the branch the predicates select.
  extent_record inconsistent{};
  inconsistent["length"_f] = 64u;
  inconsistent["payload"_f] = u32{1};
  std::fstream discard("extent_bad.bin",
                       std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

  const auto rejected = s2s::stream_cast_be<extent_record>(discard, inconsistent);
  if(rejected.has_value())
    return 1;

  return rejected.error().failure_reason == s2s::error_reason::validation_failure ? 0 : 1;
}
```

Two rules are enforced on a `variance` at compile time: the alternatives must be
distinct types, and the `match_case` values must be unique.

### Constraining an alternative

An alternative's payload is validated by putting a constraint in its tag's pack:

```cpp
s2s::match_case<0x01, s2s::as_trivial<u32, 4_B, s2s::lte{99u}>>
```

A payload that violates it is `validation_failure` in both directions, exactly
as an ordinary field's constraint is. `failed_at` names the **`variance`**, not
the alternative: an alternative has no id of its own, so a breach inside the
third alternative of `"body"` is reported as `"body"`. Which alternative it was
is recoverable from the discriminant.

**The constraint applies to the tag's resulting type, not its element type.** On
`as_vec<u8, len_from_field<"n">, ...>` the constraint is checked against
`std::vector<u8>`, so it is a predicate over the whole container rather than
over each `u8`:

```cpp
struct even_length {
  constexpr auto operator()(const std::vector<u8>& payload) const -> bool {
    return payload.size() % 2 == 0;
  }
};

s2s::match_case<0x02, s2s::as_vec<u8, s2s::len_from_field<"n">, even_length{}>>
```

The same holds for the rest: an `as_fixed_arr<u8, 4, ...>` constraint sees
`std::array<u8, 4>`, and an `as_struct<inner, ...>` constraint sees `inner`.
Constraints are values, so anything beyond the built-ins in
[Constraints and validation](../constraints.md) is spelled as a functor like the
one above — it must be usable as a template argument, which an empty struct is.

Entries are order-independent, so the size and the constraint may be written
either way round:

```cpp
s2s::as_trivial<u32, s2s::lte{99u}, 4_B>   // the same alternative
```

Two entries of the same kind in one pack is a compile error.

### Constraining the resolved variant

A `variance` takes a constraint of its own, after the deducer:

```cpp
struct even_body {
  constexpr auto operator()(const std::variant<u32, u16>& body) const -> bool {
    return std::visit([](auto value) { return value % 2 == 0; }, body);
  }
};

s2s::variance<"body", guide, even_body{}>
```

This is a different thing from the section above, and the difference is what it
sees. An alternative's constraint is handed one alternative's payload and runs
only when that alternative is the one selected. A `variance`'s constraint is
handed the resolved `std::variant` and runs whichever alternative was selected —
so it is where a rule that holds across the whole union belongs, rather than
being repeated on every tag.

Both may be declared, and both then apply: the alternative's runs against the
payload, the union's against the variant that payload was moved into. Neither
implies the other, and either can reject on its own.

Because the value is a `std::variant`, the constraint is always a functor: a
variant is not a structural type, so it cannot be a template argument to `eq{}`
and the built-in comparisons are unspellable here.

The union's pack admits a constraint and nothing else. A union's own size is
`size_dont_care` and it drives no allocation of its own — only its alternatives
do — so a size or a `max_bytes` entry here is a compile error rather than an
entry that would quietly do nothing. Two constraints is a compile error too, for
the same reason it is on any other pack.

### Bounding an alternative

A container alternative's length comes off the wire, so it carries the same
`max_bytes` ceiling every other wire-sized container does — see
[allocation limits](../reading.md#allocation-limits):

```cpp
s2s::match_case<0x02, s2s::as_vec<u8, s2s::len_from_field<"n">,
                                      s2s::max_bytes<4096>>>
```

The rules are the ones that already apply to `vec_field` and `str_field`. The
ceiling is `count * sizeof(element)` and inclusive; an alternative that declares
nothing still gets `S2S_DEFAULT_MAX_BYTES` rather than being unbounded; and a
declared bound is the author's intent, so no build setting clamps it in either
direction. An over-long alternative is `excessive_length`, reported against the
`variance`.

Only `as_vec`, `as_string` and `as_vec_of_records` take one. On the other five
the extent is fixed by the tag, so nothing a stream says can change how much is
allocated and there is nothing for a ceiling to guard — a `max_bytes` there is a
compile error rather than an entry that would quietly do nothing.

All three kinds of entry compose, in any order:

```cpp
s2s::as_vec<u8, even_length{}, s2s::max_bytes<4096>, s2s::len_from_field<"n">>
```
