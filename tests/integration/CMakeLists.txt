AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src INTEGRATION_TEST_SRCS)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../mock MOCK_SRCS)

# Integration target uses real libsqlite3 (no SqliteMock).  The
# notification-internal-static archive provides rpc_port_proxy_*
# definitions so we also drop rpc_port_mock.cc to keep one definition.
# glib_mock.cc additionally hooks the access(2) and read(2) syscalls
# without a passthrough fallback for read; those calls fire from real
# sqlite3 during in-memory db open and would throw because no
# TestFixture is active in integration tests.  Drop glib_mock here so
# the production code under test runs against real libc / libglib.
LIST(FILTER MOCK_SRCS EXCLUDE REGEX ".*sqlite_mock\\.cc$")
LIST(FILTER MOCK_SRCS EXCLUDE REGEX ".*rpc_port_mock\\.cc$")
LIST(FILTER MOCK_SRCS EXCLUDE REGEX ".*glib_mock\\.cc$")

ADD_EXECUTABLE(${TARGET_NOTIFICATION_INTEGRATION_TESTS}
  ${INTEGRATION_TEST_SRCS}
  ${MOCK_SRCS}
)

TARGET_INCLUDE_DIRECTORIES(${TARGET_NOTIFICATION_INTEGRATION_TESTS} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/notification/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/notification/src
  ${CMAKE_CURRENT_SOURCE_DIR}/../mock
)

APPLY_PKG_CONFIG(${TARGET_NOTIFICATION_INTEGRATION_TESTS} PUBLIC
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
  INIPARSER_DEPS
  LIBTZPLATFORM_CONFIG_DEPS
  PKGMGR_INFO_DEPS
  RPC_PORT_DEPS
  SECURITY_MANAGER_DEPS
  SQLITE3_DEPS
  TIZEN_CORE_DEPS
  VCONF_DEPS
)

# Shares the test-only static archive with notification-internal-unittests
# (1.5-B) so we don't duplicate compilation of the production source set.
TARGET_LINK_LIBRARIES(${TARGET_NOTIFICATION_INTEGRATION_TESTS} PRIVATE
  ${TARGET_NOTIFICATION_INTERNAL_STATIC} "-ldl")

INSTALL(TARGETS ${TARGET_NOTIFICATION_INTEGRATION_TESTS}
  DESTINATION bin)
