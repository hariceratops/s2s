# Guards what issue 010 could otherwise only verify by hand once: that the
# shipped artifact is self-contained, and that it stays warning-clean for
# consumers who build with warnings enabled.

add_executable(single_header_roundtrip roundtrip.cpp)
add_dependencies(single_header_roundtrip generate_single_header)

# Deliberately only the single header — not INCLUDE_DIR.
target_include_directories(single_header_roundtrip PRIVATE ${SINGLE_HEADER_DIR})

# -Werror so a new diagnostic fails the build rather than scrolling past.
# -Wnon-template-friend is pre-existing in include/lib/metaprog/mp.hpp's
# friend-injection trick for type_id; suppressed here so the guard catches
# anything new instead of being permanently red.
target_compile_options(single_header_roundtrip PRIVATE
  -Wall -Wextra -Wpedantic -Werror -Wno-non-template-friend)

add_test(NAME single_header_roundtrip COMMAND single_header_roundtrip)

# UserGuide.md's worked example and its claims about derived fields, kept
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

# UserGuide.md's Schema section: the four-axes example.
add_executable(guide_schema_example guide_schema_example.cpp)
add_dependencies(guide_schema_example generate_single_header)
target_include_directories(guide_schema_example PRIVATE ${SINGLE_HEADER_DIR})
target_compile_options(guide_schema_example PRIVATE
  -Wall -Wextra -Wpedantic -Werror -Wno-non-template-friend)
add_test(NAME guide_schema_example COMMAND guide_schema_example)

# UserGuide.md's Schema section: the computed-values example.
add_executable(guide_computed_example guide_computed_example.cpp)
add_dependencies(guide_computed_example generate_single_header)
target_include_directories(guide_computed_example PRIVATE ${SINGLE_HEADER_DIR})
target_compile_options(guide_computed_example PRIVATE
  -Wall -Wextra -Wpedantic -Werror -Wno-non-template-friend)
add_test(NAME guide_computed_example COMMAND guide_computed_example)

# UserGuide.md's Schema section: the maybe and variance examples.
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

# UserGuide.md's Constraints and validation section.
add_executable(guide_constraints_example guide_constraints_example.cpp)
add_dependencies(guide_constraints_example generate_single_header)
target_include_directories(guide_constraints_example PRIVATE ${SINGLE_HEADER_DIR})
target_compile_options(guide_constraints_example PRIVATE
  -Wall -Wextra -Wpedantic -Werror -Wno-non-template-friend)
add_test(NAME guide_constraints_example COMMAND guide_constraints_example)

# The generated header is tracked, so it has to be byte-identical between
# runs or every slice's diff is unreviewable.
add_test(
  NAME single_header_is_deterministic
  COMMAND ${CMAKE_COMMAND}
    -DAMALGAM=${CMAKE_SOURCE_DIR}/scripts/amalgam.py
    -DINCLUDE_DIR=${INCLUDE_DIR}
    -DENTRY=${INPUT_HEADER}
    -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/check_deterministic.cmake)
