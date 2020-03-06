CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(notification C)

SET(CMAKE_SKIP_BUILD_RPATH TRUE)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Debug")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")

MESSAGE("")
MESSAGE(">>> current directory: ${CMAKE_CURRENT_SOURCE_DIR}")
MESSAGE(">>> Build type: ${CMAKE_BUILD_TYPE}")

SET(VISIBILITY "-DEXPORT_API=\"__attribute__((visibility(\\\"default\\\")))\"")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${VISIBILITY} -fvisibility=hidden")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--gc-sections -fPIC")

##########################################################
# Define Test App
##########################################################

SET(TEST-APP-SRCS
	${CMAKE_SOURCE_DIR}/notification/test-app/main.c
)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/notification/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/notification-ex/api)

INCLUDE(FindPkgConfig)
pkg_check_modules(test_app_pkgs REQUIRED glib-2.0 gthread-2.0 vconf capi-appfw-app-control libtzplatform-config)

FOREACH(flag ${test_app_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")

ADD_EXECUTABLE(notification-test-app ${TEST-APP-SRCS})
TARGET_LINK_LIBRARIES(notification-test-app ${test_app_pkgs_LDFLAGS} notification notification-ex)

#INSTALL(TARGETS notification-test-app DESTINATION bin)

