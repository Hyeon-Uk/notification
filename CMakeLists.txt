CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(notification C)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(EXEC_PREFIX "\${prefix}")
SET(LIBDIR ${LIB_INSTALL_DIR})
SET(INCLUDEDIR "\${prefix}/include/${PROJECT_NAME}")

#ADD_SUBDIRECTORY(test-app)

SET(INIT-SRCS
	./src/notification_init.c
)

SET(SRCS
	./src/notification.c
	./src/notification_noti.c
	./src/notification_ongoing.c
	./src/notification_group.c
	./src/notification_db.c
	./src/notification_db_query.h
	./src/notification_list.c
	./src/notification_status.c
	./src/notification_ipc.c
	./src/notification_setting.c
	./src/notification_setting_service.c
	./src/notification_internal.c
	./src/notification_viewer.c
	./src/notification_error.c
	./src/notification_shared_file.c
	)
SET(HEADERS-DEVEL
	./include/notification.h
	./include/notification_internal.h
	./include/notification_db.h
	./include/notification_error.h
	./include/notification_type.h
	./include/notification_list.h
	./include/notification_ongoing.h
	./include/notification_ongoing_flag.h
	./include/notification_text_domain.h
	./include/notification_status.h
	./include/notification_status_internal.h
	./include/notification_setting.h
	./include/notification_setting_internal.h
	./include/notification_ipc.h
	./include/notification_noti.h
	./include/notification_setting_service.h
	./include/notification_viewer.h
	./include/notification_shared_file.h
   )

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

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

IF (HAVE_X11)
ADD_DEFINITIONS("-DHAVE_X11")
ENDIF (HAVE_X11)

IF (HAVE_WAYLAND)
ADD_DEFINITIONS("-DHAVE_WAYLAND")
ENDIF (HAVE_WAYLAND)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRCS})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION ${MAJORVER})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES VERSION ${FULLVER})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS})

ADD_EXECUTABLE(notification_init ${INIT-SRCS})
TARGET_LINK_LIBRARIES(notification_init ${pkgs_LDFLAGS} notification "-pie")

CONFIGURE_FILE(${PROJECT_NAME}.pc.in ${PROJECT_NAME}.pc @ONLY)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_INSTALL_DIR} COMPONENT RuntimeLibraries)
INSTALL(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)

FOREACH(hfile ${HEADERS-DEVEL})
	INSTALL(FILES ${CMAKE_SOURCE_DIR}/${hfile} DESTINATION include/${PROJECT_NAME})
ENDFOREACH(hfile)

INSTALL(FILES ${CMAKE_BINARY_DIR}/11_notification-add.post DESTINATION ${SYSCONF_INSTALL_DIR}/gumd/useradd.d/)
INSTALL(TARGETS notification_init DESTINATION bin)
