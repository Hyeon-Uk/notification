#
# Phase 0 N0-T6: Integration test skeleton.
#
# Integration tests are kept *out* of the unit-test binary because they
# talk to a live notification daemon / DB and cannot be executed offline.
# We therefore emit a custom target `integration-tests` that a downstream
# CI job can invoke once the daemon is up.
#

SET(INTEGRATION_SRCS
  test_cold_start.cc
  test_bundle_reuse.cc
)

# Build the runner binary only when gtest is available (tests/bench uses
# the same opt-in). Otherwise we still expose the custom target that CI
# scripts rely on.
FIND_PACKAGE(GTest QUIET)

IF(GTest_FOUND OR TARGET GTest::gtest)
  ADD_EXECUTABLE(notification-integration-tests
    ${INTEGRATION_SRCS}
    ${CMAKE_SOURCE_DIR}/tests/mock/alloc_counter.cc
  )
  TARGET_COMPILE_DEFINITIONS(notification-integration-tests PRIVATE
    NOTI_TEST_WRAP_ALLOC=1
  )
  TARGET_LINK_OPTIONS(notification-integration-tests PRIVATE
    "-Wl,--wrap=malloc"
    "-Wl,--wrap=free"
    "-Wl,--wrap=realloc"
    "-Wl,--wrap=calloc"
  )
  TARGET_INCLUDE_DIRECTORIES(notification-integration-tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/tests/mock
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/notification/include
    ${CMAKE_SOURCE_DIR}/src/notification-ex
    ${CMAKE_SOURCE_DIR}/src/notification-ex/api
  )
  IF(TARGET GTest::gtest)
    TARGET_LINK_LIBRARIES(notification-integration-tests PRIVATE
      GTest::gtest GTest::gtest_main pthread
      ${TARGET_NOTIFICATION} ${TARGET_NOTIFICATION_EX})
  ELSE()
    TARGET_LINK_LIBRARIES(notification-integration-tests PRIVATE
      ${GTEST_LIBRARIES} pthread
      ${TARGET_NOTIFICATION} ${TARGET_NOTIFICATION_EX})
  ENDIF()

  ADD_CUSTOM_TARGET(integration-tests
    COMMAND $<TARGET_FILE:notification-integration-tests>
    DEPENDS notification-integration-tests
    COMMENT "Run notification integration tests"
  )
ELSE()
  MESSAGE(STATUS "[integration] gtest not found, emitting echo-only target")
  ADD_CUSTOM_TARGET(integration-tests
    COMMAND ${CMAKE_COMMAND} -E echo
            "integration-tests: gtest not available - skipped"
    COMMENT "Skipping integration tests (no gtest)"
  )
ENDIF()
