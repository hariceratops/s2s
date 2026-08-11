# Compile time

s2s makes two different claims about compile time. They are often run together
into "it's constexpr", which is what makes the second one look like a
precondition for using the library at all. It is not.

## The parser is generated at compile time

This one is unconditional. A schema is a type, and the code that walks it is
produced by instantiating templates over that type. There is no field-list
object to iterate at run time, no table of function pointers, no format
interpreter, and no branch on "what kind of field is this" — the traversal is
laid out by the compiler and what remains is the field-by-field work itself.

This comes with using the library at all. It holds with a plain `std::ifstream`,
with a schema full of `std::string` and `std::vector`, on gcc 13, in a debug
build. Nothing below is required for it.

## Parsing can be performed at compile time

This one is narrower and has two conditions, both of which must hold.

**The stream must be usable during constant evaluation.** In practice that means
a type deriving from `s2s::constexpr_stream`, working on `std::array` rather
than raw pointers — see [Constexpr streams](streams.md#constexpr-streams). A
`std::ifstream` cannot be one; there are no files during constant evaluation.

**The schema must have no allocating fields.** Anything needing the heap is
disqualifying:

| Disqualifies a compile-time parse | Fine at compile time |
|---|---|
| `str_field` (`std::string`) | `fixed_string_field`, `c_str_field` |
| `vec_field` (`std::vector`) | `fixed_array_field`, `c_arr_field` |
| `vector_of_records` | `array_of_records` |
| `variance` with `as_string` or `as_vec` alternatives | `variance` over trivial and record alternatives |

The fixed-size counterparts exist precisely so a schema can be made
constexpr-eligible without giving up strings and arrays.

When both conditions hold, the whole parse happens in the compiler — including
constraint checking, presence deduction and type deduction — and the result is a
value that can be `static_assert`ed on.

<!-- docs: test/single_header/guide_constexpr_example.cpp -->
```cpp
#include "s2s.hpp"

#include <array>
#include <cstddef>

using namespace s2s_literals;

using u8 = unsigned char;
using u32 = unsigned int;

// A constexpr stream is identified by deriving from s2s::constexpr_stream, and
// works on std::array rather than char* so it is usable in constant evaluation.
template <std::size_t N>
class memstream: s2s::constexpr_stream {
  std::array<u8, N>& buffer;
  std::size_t read_idx{0};
  bool ok{true};

public:
  constexpr explicit memstream(std::array<u8, N>& b): buffer(b) {}

  template <std::size_t dest_size>
  [[nodiscard]] constexpr auto read(std::array<char, dest_size>& dest,
                                    std::size_t size_to_read) -> memstream& {
    for(std::size_t i = 0; i < size_to_read; ++i) {
      if(read_idx >= N) {
        ok = false;
        return *this;
      }
      dest[i] = static_cast<char>(buffer[read_idx++]);
    }
    return *this;
  }

  [[nodiscard]] explicit constexpr operator bool() const { return ok; }
};

// A fixed-layout partition entry. No allocating fields, so this schema is
// eligible for a compile-time parse.
using partition_entry =
  s2s::struct_field_list<
    s2s::basic_field<"start_lba", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"sector_count", u32, s2s::field_size<s2s::fixed<4>>>
  >;

constexpr auto parse_it() -> std::expected<partition_entry, s2s::cast_error> {
  std::array<u8, 8> buffer{0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xd0, 0x0d};
  memstream<8> stream(buffer);
  return s2s::struct_cast_be<partition_entry>(stream);
}

// The parse, the validation and the field lookups all happen in the compiler.
constexpr auto result = parse_it();
static_assert(result);
static_assert((*result)["start_lba"_f] == 0xdeadbeef);
static_assert((*result)["sector_count"_f] == 0xcafed00d);

auto main() -> int {
  return 0;
}
```

Note what the `static_assert`s establish: not merely that the values are right,
but that they were available to the compiler. A failure here is a compile error,
not a test failure.

## Compiler support for the constexpr path

The compile-time parse needs a newer compiler than the rest of the library.

| Compiler | Runtime use | Constexpr `struct_field_list` |
|---|---|---|
| gcc 13.1 | yes | **no** |
| gcc 14+ | yes | yes |
| clang 19.1.0 | yes | yes |
| msvc v19.39 (VS 17.9) | yes | yes |

gcc 13 rejects the `static constexpr` metadata tables with *"accessing value of
'key' through a glvalue in a constant expression"*. `CMakeLists.txt` fails
configure below gcc 14 rather than letting the failure surface later as an
inscrutable template error.
