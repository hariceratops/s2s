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
