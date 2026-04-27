AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src UNITTESTS_SRCS)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../mock MOCK_SRCS)

# N7.5-B: shared_file_common.c is compiled as an OBJECT library and linked
# into both libnotification.so and libnotification-ex.so. The helpers there
# are NOT marked EXPORT_API (they are deliberately internal — only consumed
# by the C and C++ wrappers in those two .so's). The test_shared_file_common_extra
# TU calls them directly to lift line/branch coverage; rather than perturb the
# production ABI surface we re-compile the same .c file into the test binary
# itself. The OBJECT library is already PIC + visibility-default-locally, and
# the helpers are pure functions so dual-linking does not produce duplicate
# global state.
ADD_EXECUTABLE(${TARGET_NOTIFICATION_UNITTESTS}
  ${UNITTESTS_SRCS}
  ${MOCK_SRCS}
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/notification/src/shared_file_common.c
)

TARGET_INCLUDE_DIRECTORIES(${TARGET_NOTIFICATION_UNITTESTS} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/notification/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/notification-ex
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/notification-ex/api
  ${CMAKE_CURRENT_SOURCE_DIR}/../mock
)

# N0-T1: Enable allocation counter wrap hooks for PerfAware / LeakCheck
# fixtures. Only applied to the test binary; production libs are untouched.
# N4-T1: NOTIFICATION_TIDL_C_PATH lets test_tidl_lazy.cc read the .c file
# from a stable absolute path regardless of cwd at test time. Without
# this define the test SKIPs (does not fail) so out-of-tree consumers
# remain green.
TARGET_COMPILE_DEFINITIONS(${TARGET_NOTIFICATION_UNITTESTS} PRIVATE
  NOTI_TEST_WRAP_ALLOC=1
  NOTIFICATION_TIDL_C_PATH="${CMAKE_SOURCE_DIR}/src/notification/src/notification_tidl.c"
)

# N2-T5: the test binary hosts mock TUs (tests/mock/*_mock.cc) that provide
# extern "C" definitions of libsqlite3 / libaul / libtzplatform-config /
# libglib-2.0 entry points. These act as symbol interposers for the .so
# at runtime. With the project-wide -fvisibility=hidden the mock symbols
# would be emitted with STV_HIDDEN visibility and the dynamic linker
# would NOT interpose them. Opt the test binary out with
# -fvisibility=default so every extern "C" mock stays globally visible.
TARGET_COMPILE_OPTIONS(${TARGET_NOTIFICATION_UNITTESTS} PRIVATE
  "-fvisibility=default"
  # N7.5-B: -fvisibility-inlines-hidden is C++/ObjC++ only; gating with
  # COMPILE_LANGUAGE keeps it off the .c TUs (e.g. shared_file_common.c that
  # we now compile directly into the test exe for coverage).
  "$<$<COMPILE_LANGUAGE:CXX>:-fvisibility-inlines-hidden>")  # keep inline thunks hidden; mocks are not inline
TARGET_LINK_OPTIONS(${TARGET_NOTIFICATION_UNITTESTS} PRIVATE
  "-Wl,--wrap=malloc"
  "-Wl,--wrap=free"
  "-Wl,--wrap=realloc"
  "-Wl,--wrap=calloc"
  # N2-T5: the project-wide CMAKE_EXE_LINKER_FLAGS enable --gc-sections so
  # the production binaries (notification_init, notification-test-app)
  # strip dead code. The unittest binary, however, relies on mock TUs that
  # provide C-linkage overrides (sqlite3_*, aul_*, glib_*) which are only
  # referenced from inside libnotification.so at runtime — the linker
  # cannot see those cross-TU uses and would drop the mock bodies. Make
  # the test binary opt-OUT of --gc-sections to preserve the mocks.
  "-Wl,--no-gc-sections"
  # Keep all test-binary symbols dynamic so `dlsym(RTLD_DEFAULT, ...)`
  # style probes in mock_hook / alloc_counter also resolve.
  "-Wl,--export-dynamic"
)

# N4-T2: notification-ex moved to PUBLIC=(bundle,dlog,glib,gio) /
# PRIVATE=(everything else) for its installed pkg-config + INTERFACE_*
# properties. The test binary used to inherit sqlite3 / bundle / appfw-common /
# pkgmgr-info / appfw-package-manager / uuid include dirs transitively from
# libnotification-ex; now we must declare them explicitly here so the test
# TUs that include <sqlite3.h>, <bundle.h>, etc. still compile.
APPLY_PKG_CONFIG(${TARGET_NOTIFICATION_UNITTESTS} PUBLIC
  AUL_DEPS
  BUNDLE_DEPS
  CAPI_APPFW_APP_COMMON_DEPS
  CAPI_APPFW_APP_CONTROL_DEPS
  CAPI_APPFW_PACKAGE_MANAGER_DEPS
  DLOG_DEPS
  GIO_DEPS
  GIO_UNIX_DEPS
  GLIB_DEPS
  GMOCK_DEPS
  LIBTZPLATFORM_CONFIG_DEPS
  PKGMGR_INFO_DEPS
  RPC_PORT_DEPS
  SECURITY_MANAGER_DEPS
  SQLITE3_DEPS
  TIZEN_CORE_DEPS
  UUID_DEPS
  VCONF_DEPS
)

TARGET_LINK_LIBRARIES(${TARGET_NOTIFICATION_UNITTESTS} PRIVATE
  ${TARGET_NOTIFICATION} ${TARGET_NOTIFICATION_EX} "-ldl")

# N2-T8 F2: the mock TUs (tests/mock/*_mock.cc) interpose libc / libaul /
# libsqlite3 / libglib entry points so tests can observe calls made from
# inside libnotification.so at runtime. LTO on the unittest executable
# lets the linker's IPA pass see through the interposer boundary, inline
# `aul_app_get_pkgid_bypid` into call sites as constant-return stubs, and
# thus silently skips the MOCK_HOOK bookkeeping the EXPECT_CALL assertions
# depend on (result: "never called - unsatisfied" on every mock). We opt
# the test binary out of LTO while keeping the production shared libs
# fully optimized. No effect on binary-size gate (test exe is not shipped).
SET_TARGET_PROPERTIES(${TARGET_NOTIFICATION_UNITTESTS} PROPERTIES
  INTERPROCEDURAL_OPTIMIZATION FALSE)

INSTALL(TARGETS ${TARGET_NOTIFICATION_UNITTESTS} DESTINATION bin)
