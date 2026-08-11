# Project: s2s (struct_cast)

## Context
s2s is a header-only C++23 library that declaratively converts a binary
stream into a "meta-struct" with a map-like interface — a serde for binary
formats, implemented as an embedded DSL powered by template metaprogramming.
Field kinds include trivials, arrays and length-prefixed vectors of trivials
and records, fixed and length-prefixed strings, optionals, unions, and magic
values, with in-place field validation and compile-time endianness handling.

Target is hosted C++23; there is no hardware or RTOS dependency.
Supported toolchains are gcc 13.1, clang 19.1.0, and msvc v19.39 (VS 17.9),
but gcc 13 covers the runtime path only — constexpr use of `struct_field_list`
requires gcc 14 or newer, and `CMakeLists.txt` fails configure below it.
Sources live under `include/` and are amalgamated by `scripts/amalgam.py`
into the shipped `single_header/s2s.hpp`, which is what consumers include.
Both directions are complete: stream to struct via `struct_cast_le`/`_be`, and
struct to stream via `struct_write_le`/`_be`.
Tests use GoogleTest, fetched at configure time via CMake `FetchContent`,
and are split into `test/runtime/` and `test/constexpr/`.
There is no `.clang-format` or `.clang-tidy` in the repo, so formatting
follows the surrounding file rather than a tool.

Documentation lives in `README.md` (a pitch) and `docs/` (everything else),
built as an mkdocs-material site — `mkdocs.yml` holds the nav,
`requirements-docs.txt` the one dependency, and `.github/workflows/docs.yml`
builds on every PR and deploys from main. `UserGuide.md` is a stub pointing at
`docs/`, kept only so existing links resolve. Nothing is documented twice; the
site owns the schema language, the constraint DSL, both directions, streams and
the compile-time claims.

Prose addresses the reader in neutral third person, not as "you".

Every complete program shown in `README.md` or `docs/` is backed by a source
under `test/single_header/`, registered as a CTest target, and bound to its
fenced block: an `<!-- docs: <path> -->` comment above the fence, and a
`// docs-begin` / `// docs-end` region in the source. The two must match line
for line, and `ctest` enforces it (`doc_examples_match`), alongside every page
appearing in the nav (`docs_nav_lists_every_page`). Adding an example means
adding the target and the binding, not just the prose.

Examples use realistic wire formats and real names rather than `our_struct` and
`foo`, and read or write through `std::fstream`/`std::ifstream` opened with
`std::ios::binary` rather than a `std::stringstream`.

## Overrides

- **Errors: `std::expected`, never int codes or exceptions.** The default
  for this developer's C/C++ projects is explicit error returns as
  errno-style int codes or typed enums. s2s instead returns
  `std::expected` throughout: `rw_result` (= `std::expected<void,
  error_reason>`) for internal read/write steps, `cast_result` (=
  `std::expected<void, cast_error>`) for cast steps, and
  `std::expected<T, cast_error>` from the public API such as
  `struct_cast_le`/`struct_cast_be`. Failures are raised with
  `std::unexpected(error_reason::…)`. Justified by the C++23 baseline in
  `CMakeLists.txt` and by monadic error propagation through deep template
  call chains, where int codes would have to be threaded by hand. See
  `include/error/cast_error.hpp`.

- **Trailing return types on every function.** Functions are declared
  `constexpr auto f(args) -> ret`, not `ret f(args)`, uniformly across
  `include/`. Public API entry points additionally carry `[[nodiscard]]`.

- **`constexpr` by default, with a parallel constexpr-stream path.**
  Everything that can be `constexpr` is, and stream access is written twice:
  once against `identified_as_constexpr_stream` (buffer-based, usable in
  constant evaluation) and once against runtime `readable`/`writeable`
  streams. New stream-touching code is expected to provide both overloads
  and to be covered by both `test/runtime/` and `test/constexpr/`.

- **Concepts, not SFINAE.** Templates are constrained with named concepts
  (`field_list_like`, `input_stream_like`, `trivial`,
  `variable_sized_buffer_like`, …) rather than `enable_if` or tag dispatch.
  New constraints get a named concept in the relevant header.

- **Include style: guards and relative paths.** Headers use
  `#ifndef _NAME_HPP_` / `#define _NAME_HPP_` include guards rather than
  `#pragma once`, and include each other by relative path
  (`#include "../error/cast_error.hpp"`). This is what `scripts/amalgam.py`
  expects when flattening `include/` into the single header.

- **Hand-rolled vocabulary types in `include/lib/`.** The project prefers
  its own `optional`, containers, and metaprogramming helpers over the std
  equivalents in template-facing code, specifically to keep compiler error
  messages readable. Reach for `include/lib/` before `<optional>` etc. in
  any code that participates in template deduction.

## Domains
None. Despite being usable on embedded targets, s2s itself is host-side
library code — the embedded domain rules do not apply to it.
