IF(COVERAGE)
  INSTALL(PROGRAMS
    ${CMAKE_CURRENT_SOURCE_DIR}/verify_coverage.sh
    DESTINATION bin/coverage
  )

  ADD_TEST(NAME coverage-gate
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/verify_coverage.sh
            "${CMAKE_BINARY_DIR}" "100"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  SET_TESTS_PROPERTIES(coverage-gate PROPERTIES
    DEPENDS "notification-unittests")
ENDIF(COVERAGE)
