AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src INTERNAL_TEST_SRCS)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../mock MOCK_SRCS)

# rpc_port_mock.cc defines extern "C" wrappers for rpc_port_proxy_*.
# notification-internal-static (this target's link dep) ships
# notification_tidl_proxy.c which already defines those symbols.
# Exclude the rpc-port mock here so the linker sees a single
# definition; consumer tests in 1.5-E that need to mock the proxy
# layer can re-introduce it via an opt-in CMake var.
LIST(FILTER MOCK_SRCS EXCLUDE REGEX ".*rpc_port_mock\\.cc$")

ADD_EXECUTABLE(${TARGET_NOTIFICATION_INTERNAL_UNITTESTS}
  ${INTERNAL_TEST_SRCS}
  ${MOCK_SRCS}
)

TARGET_INCLUDE_DIRECTORIES(${TARGET_NOTIFICATION_INTERNAL_UNITTESTS} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/notification/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/notification/src
  ${CMAKE_CURRENT_SOURCE_DIR}/../mock
)

APPLY_PKG_CONFIG(${TARGET_NOTIFICATION_INTERNAL_UNITTESTS} PUBLIC
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

# Links the test-only static archive (visibility=default) instead of
# the shipped libnotification.so (visibility=hidden).  Phase 1.5-B.
TARGET_LINK_LIBRARIES(${TARGET_NOTIFICATION_INTERNAL_UNITTESTS} PRIVATE
  ${TARGET_NOTIFICATION_INTERNAL_STATIC} "-ldl")

INSTALL(TARGETS ${TARGET_NOTIFICATION_INTERNAL_UNITTESTS}
  DESTINATION bin)
