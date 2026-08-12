# s2s

A declarative binary parser — a serde for binary formats — that converts a
stream into a meta-struct with a map-like interface, and back again.
Implemented as an embedded DSL powered by C++23 template metaprogramming.

## How this guide is organised

The schema is the spine. It is declared once and drives both directions, so it
is documented once:

- **[Schema](schema/index.md)** and **[Constraints](constraints.md)** are the
  shared vocabulary. Nothing in them belongs to reading or to writing.
- **[Reading](reading.md)** and **[Writing](writing.md)** cover only what each
  direction does with that vocabulary.
- **[Streams](streams.md)**, **[Compile time](compile-time.md)** and
  **[Errors](errors.md)** cut across both.
- **[Reference](reference.md)** holds the exact declarations and the known
  limitations.

## The two compile-time claims

s2s makes two claims about compile time, and they are often run together into
"it's constexpr", which makes the narrower one look like a precondition for
using the library at all. It is not.

**The parser is generated at compile time.** This is unconditional. A schema is
a type, and the traversal over it is produced by instantiating templates — no
runtime dispatch over the field list, no reflection, no format interpreter. It
holds with a plain `std::ifstream` and with a schema full of `std::string` and
`std::vector`.

**Parsing can additionally be performed at compile time.** This is narrower and
carries two conditions: a stream usable during constant evaluation, and a parsed
value that carries no heap memory out of it. [Compile time](compile-time.md)
covers both.

## Getting the library

s2s is header-only. `single_header/s2s.hpp` is the shipped artifact and is the
only file a consumer needs.

## Requirements

C++23 or newer. Compiler support differs between the runtime path and the
compile-time one:

| Compiler | Runtime use | Constexpr `struct_field_list` |
|---|---|---|
| gcc 13.1 | yes | **no** |
| gcc 14+ | yes | yes |
| clang 19.1.0 | yes | yes |
| msvc v19.39 (VS 17.9) | yes | yes |

gcc 13 rejects the `static constexpr` metadata tables with *"accessing value of
'key' through a glvalue in a constant expression"*. `CMakeLists.txt` fails
configure below gcc 14 rather than letting that surface later as a template
error.
