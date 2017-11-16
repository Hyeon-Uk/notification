CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(notification C)

#ADD_SUBDIRECTORY(test-app)

SET(INIT-SRCS
	./src/notification_init.c
)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
AUX_SOURCE_DIRECTORY(src SRCS)
LIST(REMOVE_ITEM SRCS ${INIT-SRCS})

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
	sqlite3
	vconf
	bundle
	dlog
	aul
	dbus-1
	dbus-glib-1
	capi-appfw-app-control
	capi-appfw-package-manager
	pkgmgr-info
	libtzplatform-config
	glib-2.0
	gio-2.0
	iniparser
	security-manager
	libsmack
)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -fvisibility=hidden -g -Wall")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fPIE")

SET(CMAKE_SKIP_BUILD_RPATH TRUE)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRCS})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION ${MAJORVER})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES VERSION ${FULLVER})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS})

ADD_EXECUTABLE(notification_init ${INIT-SRCS})
TARGET_LINK_LIBRARIES(notification_init ${pkgs_LDFLAGS} notification "-pie")

CONFIGURE_FILE(${PROJECT_NAME}.pc.in ${PROJECT_NAME}.pc @ONLY)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_INSTALL_DIR} COMPONENT RuntimeLibraries)
INSTALL(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
        DESTINATION include/notification
        FILES_MATCHING
		PATTERN "*_private.h" EXCLUDE
		PATTERN "*_group.h" EXCLUDE
		PATTERN "*_debug.h" EXCLUDE
        PATTERN "*.h")

INSTALL(FILES ${CMAKE_BINARY_DIR}/11_notification-add.post DESTINATION ${SYSCONF_INSTALL_DIR}/gumd/useradd.d/)
INSTALL(TARGETS notification_init DESTINATION bin)
