# Generate the amalgamated header twice and require the results to match.
foreach(run 1 2)
  execute_process(
    COMMAND python3 ${AMALGAM} ${INCLUDE_DIR} ${ENTRY} ${WORK_DIR}/determinism_${run}.hpp
    RESULT_VARIABLE status
    OUTPUT_QUIET)
  if(NOT status EQUAL 0)
    message(FATAL_ERROR "amalgam.py failed on run ${run}")
  endif()
endforeach()

file(READ ${WORK_DIR}/determinism_1.hpp first)
file(READ ${WORK_DIR}/determinism_2.hpp second)
if(NOT first STREQUAL second)
  message(FATAL_ERROR "amalgam.py output differs between runs")
endif()

# Machine-independent: no path from the generating machine may leak in.
if(first MATCHES "${INCLUDE_DIR}")
  message(FATAL_ERROR "generated header contains absolute paths from this machine")
endif()
