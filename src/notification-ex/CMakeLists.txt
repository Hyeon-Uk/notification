SET(PREFIX "${CMAKE_INSTALL_PREFIX}")
SET(EXEC_PREFIX "${PREFIX}")
SET(LIBDIR ${LIB_INSTALL_DIR})
SET(INCLUDEDIR "${PREFIX}/include/${TARGET_NOTIFICATION_EX}")
SET(VERSION_MAJOR 1)
SET(VERSION "${VERSION_MAJOR}.0.0")

AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR} NOTIFICATION_EX_SRCS)

# N2-T6: share the shared_file_common object with libnotification.so so
# five duplicated helpers (last_index_of / make_dir / is_private_path /
# copy_file / get_data_path) are emitted exactly once across both
# libraries. The include dir for shared_file_common.h lives under
# src/notification/src, which we also make visible here.
ADD_LIBRARY(${TARGET_NOTIFICATION_EX} SHARED
  ${NOTIFICATION_EX_SRCS}
  $<TARGET_OBJECTS:shared_file_common>)

SET_TARGET_PROPERTIES(${TARGET_NOTIFICATION_EX} PROPERTIES
  SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${TARGET_NOTIFICATION_EX} PROPERTIES
  VERSION ${VERSION})

TARGET_INCLUDE_DIRECTORIES(${TARGET_NOTIFICATION_EX} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../
  ${CMAKE_CURRENT_SOURCE_DIR}/api
)
# N2-T6: private (non-installed) include path so shared_file.cc can reach
# shared_file_common.h without exposing it to consumers of the public API.
TARGET_INCLUDE_DIRECTORIES(${TARGET_NOTIFICATION_EX} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../notification/src
  ${CMAKE_CURRENT_SOURCE_DIR}/../notification/include
)

APPLY_PKG_CONFIG(${TARGET_NOTIFICATION_EX} PUBLIC
  AUL_DEPS
  BUNDLE_DEPS
  CAPI_APPFW_APP_COMMON_DEPS
  CAPI_APPFW_APP_CONTROL_DEPS
  CAPI_APPFW_PACKAGE_MANAGER_DEPS
  DLOG_DEPS
  GIO_DEPS
  GIO_UNIX_DEPS
  GLIB_DEPS
  INIPARSER_DEPS
  LIBTZPLATFORM_CONFIG_DEPS
  PKGMGR_INFO_DEPS
  RPC_PORT_DEPS
  SECURITY_MANAGER_DEPS
  SQLITE3_DEPS
  TIZEN_CORE_DEPS
  UUID_DEPS
  VCONF_DEPS
)

CONFIGURE_FILE(${TARGET_NOTIFICATION_EX}.pc.in
  ${TARGET_NOTIFICATION_EX}.pc @ONLY)
SET_DIRECTORY_PROPERTIES(PROPERTIES
  ADDITIONAL_MAKE_CLEAN_FILES "${TARGET_NOTIFICATION_EX}.pc")
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_NOTIFICATION_EX}.pc
  DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)

INSTALL(TARGETS ${TARGET_NOTIFICATION_EX} DESTINATION ${LIB_INSTALL_DIR})
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  DESTINATION include/${TARGET_NOTIFICATION_EX}
  FILES_MATCHING PATTERN "*.h"
  # N3-T1 D5/D7: internal-only headers — do NOT leak to package consumers.
  # appid_lru.h is the fixed-slot LRU wrapper behind util::GetAppId; its
  # API is a detail helper of ex_util.cc and not part of the installed
  # notification-ex surface. abstract_item_keys.h is the rodata table of
  # bundle key string_views shared between abstract_item.cc (writer) and
  # a future deserializer — also internal. small_array.h is the inline
  # SmallArray<T,N> scratch template used inside stub.cc (N3-T5 A4).
  PATTERN "appid_lru.h" EXCLUDE
  PATTERN "abstract_item_keys.h" EXCLUDE
  PATTERN "small_array.h" EXCLUDE)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/api
  DESTINATION include/${TARGET_NOTIFICATION_EX}
  FILES_MATCHING PATTERN "*.h")
