CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(notification-ex_unittests CXX)

INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(notification-ex_unittests REQUIRED
    dlog
    gmock
    capi-appfw-app-control
    glib-2.0
    aul
    security-manager
    libtzplatform-config
)

FOREACH(flag ${notification-ex_unittests_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)
SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -fvisibility=hidden -Wall")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS} -std=c++11")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O2")

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/
    ${CMAKE_SOURCE_DIR}/notification-ex/
    ${CMAKE_SOURCE_DIR}/notification-ex/api
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_CURRENT_SOURCE_DIR}/../mock
)

AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src SOURCES)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../mock MOCK_SOURCES)
AUX_SOURCE_DIRECTORY(${CMAKE_SOURCE_DIR}/notification-ex NOTIFICATION_EX_SOURCES)
ADD_EXECUTABLE(${PROJECT_NAME}
    ${SOURCES}
    ${NOTIFICATION_EX_SOURCES}
    ${MOCK_SOURCES}
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${notification-ex_unittests_LDFLAGS}
    ${pkgs_LDFLAGS}
    ${pkgs_LIBRARIES}
    gmock
    notification-ex
)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin/)
