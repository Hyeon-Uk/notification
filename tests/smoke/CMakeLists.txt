# Smoke tests for notification library
SET(SMOKE_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Install smoke test scripts
INSTALL(PROGRAMS
  ${SMOKE_TEST_DIR}/smoke_01_symbol_check.sh
  ${SMOKE_TEST_DIR}/smoke_02_binary_size.sh
  ${SMOKE_TEST_DIR}/smoke_03_load.sh
  ${SMOKE_TEST_DIR}/smoke_04_cold_start.sh
  ${SMOKE_TEST_DIR}/smoke_05_memory.sh
  ${SMOKE_TEST_DIR}/smoke_06_fd_leak.sh
  DESTINATION bin/smoke_tests
)

ADD_TEST(NAME smoke_01_symbol_check
  COMMAND ${SMOKE_TEST_DIR}/smoke_01_symbol_check.sh
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

ADD_TEST(NAME smoke_02_binary_size
  COMMAND ${SMOKE_TEST_DIR}/smoke_02_binary_size.sh
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

ADD_TEST(NAME smoke_03_load
  COMMAND ${SMOKE_TEST_DIR}/smoke_03_load.sh
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

ADD_TEST(NAME smoke_04_cold_start
  COMMAND ${SMOKE_TEST_DIR}/smoke_04_cold_start.sh
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

ADD_TEST(NAME smoke_05_memory
  COMMAND ${SMOKE_TEST_DIR}/smoke_05_memory.sh
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

ADD_TEST(NAME smoke_06_fd_leak
  COMMAND ${SMOKE_TEST_DIR}/smoke_06_fd_leak.sh
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
