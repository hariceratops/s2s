# Runs a ut binary built with neither UT_COMPILE_TIME_ONLY nor UT_RUN_TIME_ONLY,
# so its summary carries both counts, and fails unless every test also ran at
# compile time.
#
# ut skips a *capturing* test lambda at compile time with no diagnostic. Both
# single-mode builds stay green when that happens, and so does a regex on the
# summary text: one skipped test out of four still reads "3 compile-time". Only
# comparing the two numbers notices.

# ut's reporter writes its summary to stderr, so both streams are captured into
# one variable rather than assuming which one carries it.
execute_process(
  COMMAND ${BINARY}
  OUTPUT_VARIABLE output
  ERROR_VARIABLE output
  RESULT_VARIABLE status
)
message("${output}")

if(NOT status EQUAL 0)
  message(FATAL_ERROR "${BINARY} exited with ${status}")
endif()

if(NOT output MATCHES "tests: ([0-9]+) \\(([0-9]+) passed, ([0-9]+) failed, ([0-9]+) compile-time\\)")
  message(FATAL_ERROR "could not find a ut summary in the output of ${BINARY}")
endif()
set(total ${CMAKE_MATCH_1})
set(compile_time ${CMAKE_MATCH_4})

if(NOT total EQUAL compile_time)
  math(EXPR skipped "${total} - ${compile_time}")
  message(FATAL_ERROR
    "${compile_time} of ${total} tests ran at compile time (${skipped} "
    "skipped). The usual cause is a test lambda that captures.")
endif()
