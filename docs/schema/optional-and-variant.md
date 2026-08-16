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
| `as_struct<T>` | a nested schema |
| `as_fixed_arr<T, N>` | `std::array<T, N>` |
| `as_arr_of_records<T, N>` | `std::array<T, N>` of schemas |
| `as_vec<T, opts...>` | `std::vector<T>` |
| `as_vec_of_records<T, opts...>` | `std::vector<T>` of schemas |
| `as_string<opts...>` | `std::string` |
| `as_fixed_string<N>` | `fixed_string<N>` |

A tag's trailing `opts...` is the same order-independent option pack its mirror
descriptor takes — a size today, and nothing else yet. `as_vec<u8,
len_from_field<"n">>` reads the same as it always did; what changed is that the
size is a pack entry rather than a fixed second argument, which is what lets
further options join it without a new spelling.

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

Two constraints are enforced on a `variance` at compile time: the alternatives
must be distinct types, and the `match_case` values must be unique.
