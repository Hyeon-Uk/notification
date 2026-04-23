CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

PROJECT(notification)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
IF(NOT ASAN_ENABLED)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
ENDIF(NOT ASAN_ENABLED)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error=deprecated-declarations")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-zdefs")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
# N2-T5: per-function / per-data sections for link-time gc-sections pruning.
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_C_FLAGS_RELEASE "-O2")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_C_FLAGS} -std=c++23")
# N2-T5: C++ needs explicit -fvisibility=hidden (EXTRA_C_FLAGS may not carry it);
# -fvisibility-inlines-hidden strips inline thunks that leak from vtables.
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O2")

# N2-T5: let the linker drop unreferenced sections and DT_NEEDED entries.
SET(CMAKE_SHARED_LINKER_FLAGS
  "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections -Wl,--as-needed")
SET(CMAKE_EXE_LINKER_FLAGS
  "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections -Wl,--as-needed")

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

SET(TARGET_NOTIFICATION "notification")
SET(TARGET_NOTIFICATION_EX "notification-ex")
SET(TARGET_NOTIFICATION_INIT "notification_init")
SET(TARGET_NOTIFICATION_TEST_APP "notification-test-app")
SET(TARGET_NOTIFICATION_UNITTESTS "notification-unittests")

ENABLE_TESTING()
ADD_TEST(NAME ${TARGET_NOTIFICATION_UNITTESTS}
  COMMAND ${TARGET_NOTIFICATION_UNITTESTS}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests)

INCLUDE(FindPkgConfig)
INCLUDE(ApplyPkgConfig)

PKG_CHECK_MODULES(AUL_DEPS REQUIRED aul)
PKG_CHECK_MODULES(BUNDLE_DEPS REQUIRED bundle)
PKG_CHECK_MODULES(CAPI_APPFW_APP_COMMON_DEPS REQUIRED capi-appfw-app-common)
PKG_CHECK_MODULES(CAPI_APPFW_APP_CONTROL_DEPS REQUIRED capi-appfw-app-control)
PKG_CHECK_MODULES(CAPI_APPFW_PACKAGE_MANAGER_DEPS REQUIRED
  capi-appfw-package-manager)
PKG_CHECK_MODULES(LIBTZPLATFORM_CONFIG_DEPS REQUIRED libtzplatform-config)
PKG_CHECK_MODULES(DLOG_DEPS REQUIRED dlog)
PKG_CHECK_MODULES(GIO_DEPS REQUIRED gio-2.0)
PKG_CHECK_MODULES(GIO_UNIX_DEPS REQUIRED gio-unix-2.0)
PKG_CHECK_MODULES(GLIB_DEPS REQUIRED glib-2.0)
PKG_CHECK_MODULES(GMOCK_DEPS REQUIRED gmock)
PKG_CHECK_MODULES(INIPARSER_DEPS REQUIRED iniparser)
PKG_CHECK_MODULES(PKGMGR_INFO_DEPS REQUIRED pkgmgr-info)
PKG_CHECK_MODULES(RPC_PORT_DEPS REQUIRED rpc-port)
PKG_CHECK_MODULES(SECURITY_MANAGER_DEPS REQUIRED security-manager)
PKG_CHECK_MODULES(SQLITE3_DEPS REQUIRED sqlite3)
PKG_CHECK_MODULES(TIZEN_CORE_DEPS REQUIRED tizen-core)
PKG_CHECK_MODULES(UUID_DEPS REQUIRED uuid)
PKG_CHECK_MODULES(VCONF_DEPS REQUIRED vconf)

ADD_SUBDIRECTORY(scripts)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(tests)

# Phase 0 N0-T2: optional benchmark harness. google/benchmark may not ship
# in every rootfs; we default OFF and skip gracefully when it is absent.
OPTION(ENABLE_BENCH "Build notification micro-benchmarks (requires google/benchmark)" OFF)
IF(ENABLE_BENCH)
  ADD_SUBDIRECTORY(tests/bench)
ENDIF()

# Phase 0 N0-T6: integration test skeleton. Daemon / DB dependent; default
# OFF so offline unit test build stays clean. Enable with -DENABLE_INTEGRATION=ON.
OPTION(ENABLE_INTEGRATION "Build notification integration tests" OFF)
IF(ENABLE_INTEGRATION)
  ADD_SUBDIRECTORY(tests/integration)
ENDIF()
