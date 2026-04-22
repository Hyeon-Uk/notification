#
# Phase 0 N0-T2: bench harness skeleton.
#
# We try to locate Google Benchmark via find_package(). If it is not
# available we leave the targets undefined and emit a STATUS message so the
# outer build continues. ENABLE_BENCH=ON is already a precondition to reach
# this file.
#

FIND_PACKAGE(benchmark QUIET)
IF(NOT benchmark_FOUND)
  # Try via pkg-config as a fallback (some distros ship benchmark.pc only).
  PKG_CHECK_MODULES(BENCHMARK_PC QUIET benchmark)
ENDIF()

IF(NOT benchmark_FOUND AND NOT BENCHMARK_PC_FOUND)
  MESSAGE(STATUS
    "[bench] Google Benchmark not found, skipping bench targets. "
    "Install pkg-config(benchmark) or build google/benchmark and re-run cmake.")
  RETURN()
ENDIF()

MESSAGE(STATUS "[bench] Google Benchmark available, building bench targets.")

SET(BENCH_COMMON_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/fixtures
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/notification/include
  ${CMAKE_SOURCE_DIR}/src/notification-ex
  ${CMAKE_SOURCE_DIR}/src/notification-ex/api
)

# Helper macro keeps five entries in one place.
MACRO(_ADD_BENCH name)
  ADD_EXECUTABLE(${name} ${name}.cc)
  TARGET_INCLUDE_DIRECTORIES(${name} PRIVATE ${BENCH_COMMON_INCLUDES})
  IF(benchmark_FOUND)
    TARGET_LINK_LIBRARIES(${name} PRIVATE benchmark::benchmark pthread)
  ELSE()
    TARGET_INCLUDE_DIRECTORIES(${name} PRIVATE ${BENCHMARK_PC_INCLUDE_DIRS})
    TARGET_LINK_DIRECTORIES(${name} PRIVATE ${BENCHMARK_PC_LIBRARY_DIRS})
    TARGET_LINK_LIBRARIES(${name} PRIVATE ${BENCHMARK_PC_LIBRARIES} pthread)
  ENDIF()
  # Link the notification shared libraries (they are built by the main tree).
  TARGET_LINK_LIBRARIES(${name} PRIVATE ${TARGET_NOTIFICATION}
    ${TARGET_NOTIFICATION_EX})
ENDMACRO()

_ADD_BENCH(bench_post_path)
_ADD_BENCH(bench_serialize)
_ADD_BENCH(bench_db_hot_path)
_ADD_BENCH(bench_reporter_lifecycle)
_ADD_BENCH(bench_dbus_rtt)

# Convenience target to run everything and collect json.
ADD_CUSTOM_TARGET(bench-all
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run_bench.sh
          ${CMAKE_CURRENT_BINARY_DIR}
          ${CMAKE_CURRENT_SOURCE_DIR}/results
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS bench_post_path bench_serialize bench_db_hot_path
          bench_reporter_lifecycle bench_dbus_rtt
  COMMENT "Running all notification bench binaries and storing JSON results"
)
