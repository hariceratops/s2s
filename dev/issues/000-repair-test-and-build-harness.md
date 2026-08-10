# [fix] Repair the test and build harness

Pre-existing breakage, discovered while planning the write path. Three
defects compound: the constexpr test directory has no build wiring, so its
five test files have never been compiled, so the stale include paths inside
them and a broken umbrella header went unnoticed. Every write-path issue has
a "verified in `test/constexpr/`" acceptance criterion that cannot be
satisfied until this is fixed, so it lands before 001.

This is not caused by the write feature and should be reviewed and committed
on its own.

## Acceptance Criteria
- `test/constexpr/CMakeLists.txt` is populated. It is currently a single
  newline, while `test/CMakeLists.txt` does `add_subdirectory(constexpr)` — so
  `constexpr_read.cpp`, `containers.cpp`, `field_list_metadata.cpp`,
  `static_tests_1.cpp`, and `static_tests_2.cpp` are silently never built or
  run. All five are registered and run under `ctest`.
- The stale include paths in those files are corrected. Files under
  `test/constexpr/` use `../include/s2s.hpp` and `../single_header/s2s.hpp`,
  which resolve to `test/include/` and `test/single_header/` — they predate
  the move into the `constexpr/` subdirectory.
- `include/s2s.hpp` is fixed: line 11 includes
  `type_deduction/type_deduction.hpp`, which commit `b8e6108` ("Reorganise
  type_deduction_folder") deleted. Compiling anything against the
  non-amalgamated `include/` tree currently fails fatally. The include is
  repointed at the reorganised headers under `include/type_deduction/`
  (`type/`, `switch/`, `if_else_ladder/`, `utils/`) or removed if redundant.
- A test compiles against the `include/` tree directly, not only against
  `single_header/s2s.hpp`, so this class of breakage cannot recur silently.
  `amalgam.py` resolves includes from its own file map and skips missing ones,
  which is why the generated header still compiles cleanly and why nothing
  caught this.
- **The build requires a compiler that can actually compile the constexpr
  path.** CMake pins no compiler, so it picks the default `c++` — gcc 13.3 on
  this machine — which cannot compile constexpr use of `struct_field_list`: a
  bare `constexpr` instantiation produces 9 errors of the form
  `accessing value of 'key' through a 'std::pair<std::string_view,
  field_type_info>' glvalue in a constant expression`, in
  `field_list_metadata`'s `static constexpr` table initializers. The same
  translation unit compiles with **0 errors under g++-14**. The runtime path
  builds fine on 13.3; only the constexpr path needs 14. Either raise the
  minimum in `CMakeLists.txt` and fail configure below it, or document it —
  but `README.md` and `AGENTS.md` currently claim gcc 13.1 as the minimum,
  which is understated for constexpr use and should be corrected.
- **The stale constexpr test files are updated to the current API.** Under
  g++-14, `constexpr_read.cpp` still fails with 15 errors because it
  references `s2s::struct_tag`, removed when commit `5d2ff8b` ("Renaming
  type_tags as as_xxx") renamed the tag types. This is ordinary API drift in
  files that have never been compiled. Each of the five files is either
  updated to the current API or — if obsolete — deleted with the reason
  stated in the commit.
- `ctest` reports a nonzero constexpr test count where it previously reported
  none, and the constexpr tests pass rather than merely building.
- **Deferred, not done — `static_tests_1.cpp`.** It is written against an API
  several generations old (`field<"x", int, 4>` where `field` now takes
  `<id, T, field_size<...>, constraint>`; `typelist::typelist` for
  `typelist::list`) and asserts over traits in
  `include/field_validation/field_value_constraints_traits.hpp`, which **no
  header includes** — so those traits never reach the amalgamated single
  header. Repairing it means rewriting the assertions against the current API
  *and* deciding whether that orphaned header gets wired in or deleted. That
  is the "Static test cases for all type traits and concepts" and "Remove dead
  code" items in `doc/todos.md`, not harness repair. It is left unregistered
  with a TODO in `test/constexpr/CMakeLists.txt` explaining why.
