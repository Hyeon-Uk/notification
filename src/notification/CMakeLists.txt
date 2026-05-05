SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(EXEC_PREFIX ${CMAKE_INSTALL_PREFIX})
SET(LIBDIR ${LIB_INSTALL_DIR})
SET(INCLUDEDIR "${PREFIX}/include/${TARGET_NOTIFICATION}")

AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src NOTIFICATION_SRCS)
ADD_LIBRARY(${TARGET_NOTIFICATION} SHARED ${NOTIFICATION_SRCS})

SET_TARGET_PROPERTIES(${TARGET_NOTIFICATION} PROPERTIES SOVERSION ${MAJORVER})
SET_TARGET_PROPERTIES(${TARGET_NOTIFICATION} PROPERTIES VERSION ${FULLVER})

# Phase 2 .c -> .cc migration:  CMAKE_C_FLAGS sets -fvisibility=hidden
# globally for all .c TUs, but CMAKE_CXX_FLAGS does NOT (applying it
# globally breaks mock-extern-"C"-wrapper resolution in tests).  Apply
# the visibility flag here per-target on the SHARED library only, so
# .cc files compiled into libnotification.so still get the same hidden
# default they had as .c files.  EXPORT_API-marked symbols stay
# exported via __attribute__((visibility("default"))).
#
# Phase 3 Half-A closed the per-file RAII migration; -fpermissive
# is no longer needed.  Implicit-conversion call sites that the
# C-to-C++ migration originally exposed are now either
# explicitly cast at the call site or wrapped via raii/*.hh
# unique_ptr aliases.  If any new -fpermissive-only conversions
# slip back in, the build will fail at this target -- which is
# the desired contract.
#
# Phase 4 / 4.1 adds -ffunction-sections + -fdata-sections so the
# linker can drop unreferenced functions and data via --gc-sections
# (set in TARGET_LINK_OPTIONS below).  Scoped to this SHARED library
# only -- the internal-static archive intentionally retains every
# symbol so tests/internal can reach the non-EXPORT_API surface.
TARGET_COMPILE_OPTIONS(${TARGET_NOTIFICATION} PRIVATE
    -fvisibility=hidden
    -ffunction-sections
    -fdata-sections
    # Phase 4 / 4.1b: merge identical string and numeric constants
    # across TUs.  Allowed by C++ (and safe in our codebase --
    # we do not do pointer-identity comparisons on string literals).
    # Typically nets a few KB by deduplicating ERR/WARN/DBG format
    # strings that appear in many .cc files.
    -fmerge-all-constants
    # Phase 4 / 4.1b: hide inline function definitions from the
    # dynamic-symbol table.  EXPORT_API explicit-default markings
    # are unaffected.  Reduces template-instantiation duplication
    # leaking into .dynsym (e.g. unique_ptr<>::~unique_ptr<>).
    # CXX-only via generator expression -- the lone .c TU
    # (notification_tidl_proxy.c) would error out under
    # `-Werror` because cc1 rejects this flag for C.
    $<$<COMPILE_LANGUAGE:CXX>:-fvisibility-inlines-hidden>)

# Phase 4 / 4.1: dead-section elimination at link time.  Combines
# with the per-section compile flags above to drop functions and
# data that no other reachable section references.  EXPORT_API
# symbols stay anchored via .dynsym so they are never collected.
TARGET_LINK_OPTIONS(${TARGET_NOTIFICATION} PRIVATE
    -Wl,--gc-sections)

TARGET_INCLUDE_DIRECTORIES(${TARGET_NOTIFICATION} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

APPLY_PKG_CONFIG(${TARGET_NOTIFICATION} PUBLIC
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
  VCONF_DEPS
)

# Test-only static archive: same source set as ${TARGET_NOTIFICATION}
# but compiled with -fvisibility=default so non-EXPORT_API helpers
# become reachable from internal-API unit-test translation units.
# Not installed; consumed only by tests/internal/.
#
# Source set is identical to libnotification.so (visibility is the
# only delta).  The tidlc-generated notification_tidl_proxy.c stays
# in to keep the rpc_port_proxy_noti_service_* / _notification_*
# symbols defined; tests/internal excludes rpc_port_mock.cc from its
# mock glob so the test executable does not see duplicate
# definitions of those symbols.
ADD_LIBRARY(${TARGET_NOTIFICATION_INTERNAL_STATIC} STATIC
  ${NOTIFICATION_SRCS})

TARGET_COMPILE_OPTIONS(${TARGET_NOTIFICATION_INTERNAL_STATIC} PRIVATE
  -fvisibility=default -fPIC)

TARGET_INCLUDE_DIRECTORIES(${TARGET_NOTIFICATION_INTERNAL_STATIC} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

APPLY_PKG_CONFIG(${TARGET_NOTIFICATION_INTERNAL_STATIC} PUBLIC
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
  VCONF_DEPS
)

CONFIGURE_FILE(${TARGET_NOTIFICATION}.pc.in
  ${TARGET_NOTIFICATION}.pc @ONLY)
INSTALL(TARGETS ${TARGET_NOTIFICATION}
  DESTINATION ${LIB_INSTALL_DIR} COMPONENT RuntimeLibraries)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NOTIFICATION}.pc
  DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
  DESTINATION include/notification
  FILES_MATCHING
  PATTERN "*_private.h" EXCLUDE
  PATTERN "*_group.h" EXCLUDE
  PATTERN "*_debug.h" EXCLUDE
  PATTERN "*_tidl.h" EXCLUDE
  PATTERN "*_tidl_proxy.h" EXCLUDE
  PATTERN "*.h"
)
