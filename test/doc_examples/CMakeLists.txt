# Every complete program the documentation shows is backed by a source here,
# compiled and run, and checked against the fenced block it came from.

# docs/writing.md's claims about derived fields, kept
# compilable so the docs cannot drift from the library unnoticed.
add_executable(doc_example doc_example.cpp)
add_dependencies(doc_example generate_single_header)
target_include_directories(doc_example PRIVATE ${SINGLE_HEADER_DIR})
target_compile_options(doc_example PRIVATE
  -Wall -Wextra -Wpedantic -Werror -Wno-non-template-friend)
add_test(NAME doc_example COMMAND doc_example)

# README.md's write-direction example, kept compilable for the same reason as
# doc_example: the README shipped a wrong include path for months because
# nothing compiled the block it sat in.
add_executable(readme_roundtrip_example readme_roundtrip_example.cpp)
add_dependencies(readme_roundtrip_example generate_single_header)
target_include_directories(readme_roundtrip_example PRIVATE ${SINGLE_HEADER_DIR})
target_compile_options(readme_roundtrip_example PRIVATE
  -Wall -Wextra -Wpedantic -Werror -Wno-non-template-friend)
add_test(NAME readme_roundtrip_example COMMAND readme_roundtrip_example)

# docs/schema/ pages: the four-axes example.
add_executable(guide_schema_example guide_schema_example.cpp)
add_dependencies(guide_schema_example generate_single_header)
target_include_directories(guide_schema_example PRIVATE ${SINGLE_HEADER_DIR})
target_compile_options(guide_schema_example PRIVATE
  -Wall -Wextra -Wpedantic -Werror -Wno-non-template-friend)
add_test(NAME guide_schema_example COMMAND guide_schema_example)

# docs/schema/ pages: the computed-values example.
add_executable(guide_computed_example guide_computed_example.cpp)
add_dependencies(guide_computed_example generate_single_header)
target_include_directories(guide_computed_example PRIVATE ${SINGLE_HEADER_DIR})
target_compile_options(guide_computed_example PRIVATE
  -Wall -Wextra -Wpedantic -Werror -Wno-non-template-friend)
add_test(NAME guide_computed_example COMMAND guide_computed_example)

# docs/schema/ pages: the maybe and variance examples.
add_executable(guide_optional_example guide_optional_example.cpp)
add_dependencies(guide_optional_example generate_single_header)
target_include_directories(guide_optional_example PRIVATE ${SINGLE_HEADER_DIR})
target_compile_options(guide_optional_example PRIVATE
  -Wall -Wextra -Wpedantic -Werror -Wno-non-template-friend)
add_test(NAME guide_optional_example COMMAND guide_optional_example)

add_executable(guide_variance_example guide_variance_example.cpp)
add_dependencies(guide_variance_example generate_single_header)
target_include_directories(guide_variance_example PRIVATE ${SINGLE_HEADER_DIR})
target_compile_options(guide_variance_example PRIVATE
  -Wall -Wextra -Wpedantic -Werror -Wno-non-template-friend)
add_test(NAME guide_variance_example COMMAND guide_variance_example)

# docs/constraints.md.
add_executable(guide_constraints_example guide_constraints_example.cpp)
add_dependencies(guide_constraints_example generate_single_header)
target_include_directories(guide_constraints_example PRIVATE ${SINGLE_HEADER_DIR})
target_compile_options(guide_constraints_example PRIVATE
  -Wall -Wextra -Wpedantic -Werror -Wno-non-template-friend)
add_test(NAME guide_constraints_example COMMAND guide_constraints_example)

# docs/reading.md: the worked example.
add_executable(guide_reading_example guide_reading_example.cpp)
add_dependencies(guide_reading_example generate_single_header)
target_include_directories(guide_reading_example PRIVATE ${SINGLE_HEADER_DIR})
target_compile_options(guide_reading_example PRIVATE
  -Wall -Wextra -Wpedantic -Werror -Wno-non-template-friend)
add_test(NAME guide_reading_example COMMAND guide_reading_example)

# docs/writing.md: the worked example.
add_executable(guide_writing_example guide_writing_example.cpp)
add_dependencies(guide_writing_example generate_single_header)
target_include_directories(guide_writing_example PRIVATE ${SINGLE_HEADER_DIR})
target_compile_options(guide_writing_example PRIVATE
  -Wall -Wextra -Wpedantic -Werror -Wno-non-template-friend)
add_test(NAME guide_writing_example COMMAND guide_writing_example)

# docs/streams.md: the reference implementation.
add_executable(guide_stream_example guide_stream_example.cpp)
add_dependencies(guide_stream_example generate_single_header)
target_include_directories(guide_stream_example PRIVATE ${SINGLE_HEADER_DIR})
target_compile_options(guide_stream_example PRIVATE
  -Wall -Wextra -Wpedantic -Werror -Wno-non-template-friend)
add_test(NAME guide_stream_example COMMAND guide_stream_example)

# docs/compile-time.md: the constexpr example.
add_executable(guide_constexpr_example guide_constexpr_example.cpp)
add_dependencies(guide_constexpr_example generate_single_header)
target_include_directories(guide_constexpr_example PRIVATE ${SINGLE_HEADER_DIR})
target_compile_options(guide_constexpr_example PRIVATE
  -Wall -Wextra -Wpedantic -Werror -Wno-non-template-friend)
add_test(NAME guide_constexpr_example COMMAND guide_constexpr_example)

# README.md's read-direction example.
add_executable(readme_read_example readme_read_example.cpp)
add_dependencies(readme_read_example generate_single_header)
target_include_directories(readme_read_example PRIVATE ${SINGLE_HEADER_DIR})
target_compile_options(readme_read_example PRIVATE
  -Wall -Wextra -Wpedantic -Werror -Wno-non-template-friend)
add_test(NAME readme_read_example COMMAND readme_read_example)

# The README's read example opens a file the way a consumer would, so the file
# has to exist before it runs. Generated rather than shown, to keep that example
# a pitch instead of a setup script.
add_test(
  NAME make_doc_fixtures
  COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/make_doc_fixtures.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(make_doc_fixtures PROPERTIES FIXTURES_SETUP doc_fixtures)
set_tests_properties(readme_read_example PROPERTIES FIXTURES_REQUIRED doc_fixtures)

# Every complete program shown in README.md or docs/ must be bound to a
# source under this directory, and must still match it. The README shipped a
# wrong include path for months because nothing compiled the block it sat in;
# registering the targets above only fixes that for as long as someone
# remembers to, so the pairing is checked rather than trusted.
add_test(
  NAME doc_examples_match
  COMMAND python3 ${CMAKE_SOURCE_DIR}/scripts/check_doc_examples.py ${CMAKE_SOURCE_DIR})

# The guide used to promise sections that did not exist; split across pages the
# equivalent failure is a page nothing links to. mkdocs --strict catches nav
# entries pointing at missing files, but not files no nav entry points at.
add_test(
  NAME docs_nav_lists_every_page
  COMMAND python3 ${CMAKE_SOURCE_DIR}/scripts/check_nav.py ${CMAKE_SOURCE_DIR})
