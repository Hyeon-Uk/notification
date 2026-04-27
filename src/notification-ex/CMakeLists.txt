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

# N4-T2: PUBLIC vs PRIVATE pkg-config split.
#
# The libnotification-ex.so installed surface (notification-ex/api/*.h plus the
# non-EXCLUDE'd headers under notification-ex/) transitively includes only a
# small set of system headers — bundle.h, dlog.h, glib.h, gio/gio.h. Every
# other pkg-config dependency is consumed *only* from .cc translation units
# inside this library. Linking those as PRIVATE keeps the build correct (.cc's
# still see the includes / link the libraries) but stops CMake from forwarding
# the include dirs / -l flags to downstream consumers of TARGET_NOTIFICATION_EX
# via INTERFACE_* properties. Combined with --as-needed (set globally in the
# root CMakeLists), this prunes DT_NEEDED entries that were already linked but
# not actually referenced from downstream binaries.
#
# Removed entirely: INIPARSER_DEPS, RPC_PORT_DEPS, TIZEN_CORE_DEPS — verified
# via `grep -rn iniparser|tizen-core|rpc-port src/notification-ex/` returning
# zero hits inside the notification-ex tree (libnotification still uses the
# three on the C side, root PKG_CHECK_MODULES blocks remain for that target).
APPLY_PKG_CONFIG(${TARGET_NOTIFICATION_EX} PUBLIC
  BUNDLE_DEPS
  DLOG_DEPS
  GIO_DEPS
  GLIB_DEPS
)

APPLY_PKG_CONFIG(${TARGET_NOTIFICATION_EX} PRIVATE
  AUL_DEPS
  CAPI_APPFW_APP_COMMON_DEPS
  CAPI_APPFW_APP_CONTROL_DEPS
  CAPI_APPFW_PACKAGE_MANAGER_DEPS
  GIO_UNIX_DEPS
  LIBTZPLATFORM_CONFIG_DEPS
  PKGMGR_INFO_DEPS
  SECURITY_MANAGER_DEPS
  SQLITE3_DEPS
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
