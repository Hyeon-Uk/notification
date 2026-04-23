AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src UNITTESTS_SRCS)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../mock MOCK_SRCS)

ADD_EXECUTABLE(${TARGET_NOTIFICATION_UNITTESTS}
  ${UNITTESTS_SRCS}
  ${MOCK_SRCS}
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
TARGET_COMPILE_DEFINITIONS(${TARGET_NOTIFICATION_UNITTESTS} PRIVATE
  NOTI_TEST_WRAP_ALLOC=1
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
  "-fvisibility-inlines-hidden")  # keep inline thunks hidden; mocks are not inline
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

APPLY_PKG_CONFIG(${TARGET_NOTIFICATION_UNITTESTS} PUBLIC
  AUL_DEPS
  CAPI_APPFW_APP_CONTROL_DEPS
  DLOG_DEPS
  GIO_DEPS
  GIO_UNIX_DEPS
  GLIB_DEPS
  GMOCK_DEPS
  LIBTZPLATFORM_CONFIG_DEPS
  PKGMGR_INFO_DEPS
  RPC_PORT_DEPS
  SECURITY_MANAGER_DEPS
  TIZEN_CORE_DEPS
  VCONF_DEPS
)

TARGET_LINK_LIBRARIES(${TARGET_NOTIFICATION_UNITTESTS} PRIVATE
  ${TARGET_NOTIFICATION} ${TARGET_NOTIFICATION_EX} "-ldl")

INSTALL(TARGETS ${TARGET_NOTIFICATION_UNITTESTS} DESTINATION bin)
